#include <ncurses.h>
#include <menu.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "ui.h"
#include "config.h"

static bool display(char *sourceDir);
static void close();
static void handleKeypress(int key, char *sourceDir);
static void handleKeypress(int key, char *sourceDir); 
static bool handleSearch(); 
static void displaySearchInput(); 

//bool searchInputFocused = false;
static bool searchRow = 0;
static bool searchCol = 0;
static char *searchLabel = "Search:";

static struct Window _searchUi = {
	.window = NULL,
	.display = &display,
	.close = &close,
	.handleKeypress = &handleKeypress,
};
struct Window *searchUi = &_searchUi;

static struct Menu *content = NULL;

static bool display(char *sourceDir) {
	struct Menu *searchResults = malloc(sizeof(struct Menu));
	if(!searchResults)
		return false;

	searchResults->menu = NULL;
	searchResults->items = NULL;
	//searchResults->setItems = &setSearchItems;
	//searchResults->freeItems = &freeSearchItems;
	//searchResults->handleSelect = &handleSearchResultSelect;

	content = searchResults;
	return true;
}

static void close() {
	if(!content)
		return;

	// free content menu and such
}

bool initializeSearchUi() {
	searchUi->window = newwin(LINES - headerHeight, COLS, headerHeight, 0);
	if(searchUi->window)
		return true;
	return false;
}

static void handleKeypress(int key, char *sourceDir) {
	switch(key) {
		case 's':
			//searchInputFocused = true;
			handleSearch();
			break;
		case KEY_DOWN:
			// handle down function
			menu_driver(content->menu, REQ_DOWN_ITEM);
			break;
		case KEY_UP:
			//handle up function
			menu_driver(content->menu, REQ_UP_ITEM);
			break;
		case KEY_PPAGE:
			menu_driver(content->menu, REQ_SCR_UPAGE);
			break;
		case KEY_NPAGE:
			menu_driver(content->menu, REQ_SCR_DPAGE);
		break;
		default:
			content->handleSelect(content, key, sourceDir);
			break;
		break;
	}
}

static bool handleSearch() {
	char searchBuf[256];
	displaySearchInput();
	echo();
	wgetnstr(searchUi->window, searchBuf, sizeof(searchBuf) - 1);
	mvprintw(searchRow, searchCol + strlen(searchLabel),"%s", searchBuf);
	noecho();
	//searchInputFocused = false;
	if(searchBuf[0] == '\0')
		return false;
	return true;
}

static void displaySearchInput() {
	int width = getmaxx(searchUi->window);
	mvwprintw(searchUi->window, searchRow, 0, "%*s", width, "");
	mvwprintw(searchUi->window, searchRow, searchCol, "%s", searchLabel);
}

