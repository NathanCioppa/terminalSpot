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

size_t getMinMenuWidth(ITEM **items, size_t menuMarkLen);
bool universalControl(char key, char *sourceDir);

WINDOW *headerWINDOW;
static int nameSize = 0;
struct Window *devicesWin = NULL;
struct Window *libraryWin = NULL;

struct Window *currentWin = NULL;

void initializeUi(char *sourceDir) {
	devicesWin = malloc(sizeof(struct Window));
	libraryWin = malloc(sizeof(struct Window));

	currentWin = malloc(sizeof(struct Window));

	initscr();
	raw();
	noecho();
	refresh();
	int headerHeight = 2;
	headerWINDOW = newwin(headerHeight, COLS, 0, 0);
	drawName(sourceDir);
	wrefresh(headerWINDOW);

	devicesWin->window = newwin(LINES - headerHeight, COLS, headerHeight, 0);
	initializeDevicesWin();

	libraryWin->window = newwin(LINES - headerHeight, COLS, headerHeight, 0);
	initializeLibraryWin(sourceDir);
}

void startUi(char *sourceDir) {
	//initscr();
	//raw();
	//noecho();
	//refresh();

	libraryWin->display(sourceDir);
}

void uiLooper(char *sourceDir) {
	currentWin = libraryWin;
	keypad(currentWin->window, TRUE);
	
	int key;
	while((key = wgetch(currentWin->window))) {
		if(universalControl(key, sourceDir))
			continue; // universal controls can re-assign currentWin 

		currentWin->handleKeypress(key, sourceDir);
	}

}

bool universalControl(char key, char *sourceDir) {
	switch(key) {
		case 'd':
			currentWin->close();
			keypad(currentWin->window, FALSE);
			currentWin = devicesWin;
			currentWin->display(sourceDir);
			keypad(currentWin->window, TRUE);
			return true;
			break;
		case 'l':
			currentWin->close();
			keypad(currentWin->window, FALSE);
			currentWin = libraryWin;
			currentWin->display(sourceDir);
			keypad(currentWin->window, TRUE);
			return true;
			break;
		case 'q':
			endwin();
			exit(0);
		break;
	}
	return false;
}

bool drawName(char *sourceDir) {
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
	unsigned int height, width;
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
	post_menu(menu);
	return menu;
}

// expects **items to be NULL terminated;
size_t getMinMenuWidth(ITEM **items, size_t menuMarkLen) {
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

