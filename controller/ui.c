#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ncurses.h>
#include <menu.h>
#include <limits.h>

#include "ui.h"
#include "utils.h"
#include "config.h"
#include "spotifyCommands.h"
#include "devicesUi.h"
#include "libraryUi.h"

static bool drawName(char *sourceDir);
static size_t getMinMenuWidth(ITEM **items, size_t menuMarkLen);
static bool universalControl(char key, char *sourceDir);
static bool switchWindow(struct Window *nextWin, char *sourceDir);

WINDOW *headerWINDOW;
static int nameSize = 0;
struct Window *devicesWin = NULL;
struct Window *libraryWin = NULL;

struct Window *currentWin = NULL;
struct LazyTracker *currentLazyTracker = NULL;
struct LazyTracker *backLazyTracker = NULL;
struct Menu *backLazy = NULL;

char *lazyContext = NULL;
char *backLazyContext = NULL;

// Returns true if initialization is successful, and ncurses mode is entered.
// On a false return, ncurses mode is ended.
bool initializeUi(char *sourceDir) {
	devicesWin = malloc(sizeof(struct Window));
	libraryWin = malloc(sizeof(struct Window));

	if(!(devicesWin && libraryWin))
		return false;

	initscr();
	raw();
	noecho();
	refresh();

	int headerHeight = 2;
	headerWINDOW = newwin(headerHeight, COLS, 0, 0);
	devicesWin->window = newwin(LINES - headerHeight, COLS, headerHeight, 0);
	libraryWin->window = newwin(LINES - headerHeight, COLS, headerHeight, 0);

	bool windowsAllocated = headerWINDOW && devicesWin->window && libraryWin->window;
	if(windowsAllocated && drawName(sourceDir)) {
		wrefresh(headerWINDOW);
		initializeDevicesWin();
		initializeLibraryWin(sourceDir);
		
		return true;
	}

	free(devicesWin);
	free(libraryWin);
	endwin();

	return false;
}

bool startDefaultWindow(char *sourceDir) {
	if(libraryWin->display(sourceDir)) {
		currentWin = libraryWin;
		wrefresh(currentWin->window);
		return true;
	}
	return false;
}

void runUiLooper(char *sourceDir) {
	keypad(currentWin->window, TRUE);
	
	int key;
	while((key = wgetch(currentWin->window))) {
		if(universalControl(key, sourceDir))
			continue; // universal controls can re-assign currentWin 

		currentWin->handleKeypress(key, sourceDir);
	}

}

static bool universalControl(char key, char *sourceDir) {
	switch(key) {
		case 'd':
			if(currentWin != devicesWin)
				switchWindow(devicesWin, sourceDir);
			return true;
			break;
		case 'l':
			if(currentWin != libraryWin)
				switchWindow(libraryWin, sourceDir);
			return true;
			break;
		case 'z':
			shuffleOn();
			break;
		case 'x':
			shuffleOff();
			break;
		case 'q':
			endwin();
			exit(0);
		break;
	}
	return false;
}

static bool switchWindow(struct Window *nextWin, char *sourceDir) {
	if (!nextWin->display(sourceDir))
		return false;

	keypad(currentWin->window, FALSE);
	currentWin->close();

	keypad(nextWin->window, TRUE);
	wrefresh(currentWin->window);
	wrefresh(nextWin->window);

	currentWin = nextWin;
	return true;
}

static bool drawName(char *sourceDir) {
	char name[31]; // maximum allowed display name by spotify is 30 characters, 31 allows space for terminating char.
	if (!getName(name, sourceDir))
		return false;

	nameSize = strlen(name);
	mvwprintw(headerWINDOW, 0, 0, "%s", name);
	wrefresh(headerWINDOW);
	return true;
}

// Handles constructing and posting a menu of given items to a given window. Will not refresh window after posting. 
// At time of writing, all menus for this application should span the entire window height available below row.
// If minimize is false, menu will span full window width to the right of column.
// row and column are relative to the window position, not the top left corner of the actual terminal. 
MENU *assembleMenu(ITEM **items, WINDOW *window, unsigned int row, unsigned int column, char *menuMark, bool minimize) {
	int height, width;
	getmaxyx(window, height, width);
	height -= row;
	width -= column;

	if(minimize) {
		unsigned int minWidth = getMinMenuWidth(items, strlen(menuMark));
		if (minWidth < width)
			width = minWidth;
	}

	MENU *menu = new_menu(items);
	if(menu == NULL)
		return NULL;
	
	set_menu_mark(menu, menuMark);
	set_menu_win(menu, window);
	set_menu_sub(menu, derwin(window, height, width, row, column));
	set_menu_format(menu, height, 1);
	post_menu(menu);

	return menu;
}

// Expects **items to be NULL terminated.
// Used by assembleMenu().
static size_t getMinMenuWidth(ITEM **items, size_t menuMarkLen) {
	size_t i = 0;
	size_t longestNameLen = 0;
	size_t longestDescLen = 0;

	ITEM *thisItem;
	while((thisItem = items[i]) != NULL) {
		char *name = (char *)item_name(thisItem);
		if(name != NULL) {
			size_t len = strlen(name);
			if(len > longestNameLen)
				longestNameLen = len;
		}

		char *desc = (char *)item_description(thisItem);
		if(desc != NULL) {
			size_t len = strlen(desc);
			if(len > longestDescLen)
				longestDescLen = len;
		}
		
		i++;
	}

	size_t space = 0;
	if(longestNameLen != 0 && longestDescLen != 0)
		space = 1;

	return longestNameLen + longestDescLen + menuMarkLen + space;
}

bool setCurrentLazy(FILE *lazyInitNewLineList, struct LazyTracker *(*initFunction)(FILE *newLineList, int limitPerRequest, bool (*expand)(struct LazyTracker *self, char *sourceDir), void (*clean)(struct LazyTracker *self)), bool (*expand)(struct LazyTracker *self, char *sourceDir), void (*clean)(struct LazyTracker *self)) {
	struct LazyTracker *tracker = initFunction(lazyInitNewLineList, 50, expand, clean);
	if(!tracker)
		return false;

	tracker->expand = expand;
	tracker->clean = clean;

	currentLazyTracker = tracker;
	isEpisode = false;
	
	return true;
}

void closeCurrentLazy() {
	currentLazyTracker->clean(currentLazyTracker);
	lazyContext = NULL;
	currentLazyTracker = NULL;

	if(backLazyTracker) {
		currentLazyTracker = backLazyTracker;
		backLazyTracker = NULL;
		lazyContext = backLazyContext;
		backLazyContext = NULL;
	}
}

