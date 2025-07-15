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

static int nameSize = 0;
WINDOW *currentWin, *headerWin, *libraryWin, *searchWin, *devicesWin;

size_t getMinMenuWidth(ITEM **items, size_t menuMarkLen);

void initWindows() {
	int headerHeight = 2;
	headerWin = newwin(headerHeight, COLS, 0, 0);
	devicesWin = newwin(LINES - headerHeight, COLS, headerHeight, 0);
	libraryWin = newwin(LINES - headerHeight, COLS, headerHeight, 0);
	//keypad(devicesWin, TRUE);
	keypad(libraryWin, TRUE);
}

bool drawName(char *sourceDir) {
	char name[31]; // maximum allowed display name by spotify is 30 characters, 31 allows space for terminating char.
	if (!getName(name, sourceDir))
		return false;

	nameSize = strlen(name);
	mvwprintw(headerWin, 0, 0, "%s", name);
	wrefresh(headerWin);
	return true;
}

// Handles constructing and posting a menu of given items to a given window. Will not refresh window after posting. 
// At time of writing, all menus for this application should span the entire window height available below row.
// If minimize is false, menu will span full window width to the right of column.
// row and column are relative to the windoe position, not the top left corner of the actual terminal. 
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

