#include <ncurses.h>
#include <menu.h>
#include <stdlib.h>
#include <stdbool.h>

#include "ui.h"
#include "config.h"

static bool display(char *sourceDir);
static void close();
static void handleKeypress(int key, char *sourceDir);
static void freeFilter(struct Menu *self); 
static bool filterSetItems(struct Menu *self, char *null); 
static void handleKeypress(int key, char *sourceDir); 
static bool filterHandleSelect(struct Menu *self, int key, char *sourceDir); 

static struct Window _searchUi = {
	.window = NULL,
	.display = &display,
	.close = &close,
	.handleKeypress = &handleKeypress,
};
struct Window *searchUi = &_searchUi;

static struct Menu _filters = {
	.menu = NULL,
	.items = NULL,
	.setItems = &filterSetItems,
	.freeItems = &freeFilter,
	.handleSelect = &filterHandleSelect,
};
static struct Menu *filters = &_filters;

static struct Menu *content = NULL;
static struct Menu *activeMenu = NULL;


static bool display(char *sourceDir) {
	filters->setItems(filters, NULL);
	if(!filters->items)
		return false;

	unsigned int row = 1;
	unsigned int column = 0;
	filters->menu = assembleMenu(filters->items, searchUi->window, row, column, "", true);
	if(!filters->menu) {
		for(int i=0; filters->items[i]; i++)
			free_item(filters->items[i]);
		free(filters->items);
		filters->items = NULL;
		return false;
	}
	activeMenu = filters;
	return true;
}

static void close() {
	unpost_menu(filters->menu);
	free_menu(filters->menu);
	filters->freeItems(filters);
}

static bool filterSetItems(struct Menu *self, char *null) {
	size_t filterSize = 6;

	ITEM **filterItems = malloc(sizeof(ITEM *) * (filterSize + 1));
	if(filterItems == NULL)
		return false;

	filterItems[searchAlbumsIdx] = new_item("Albums","");
	filterItems[searchArtistsIdx] = new_item("Artists", "");
	filterItems[searchPlaylistsIdx] = new_item("Playlists","");
	filterItems[searchShowsIdx] = new_item("Shows", "");
	filterItems[searchEpisodesIdx] = new_item("Episodes", "");
	filterItems[searchSongsIdx] = new_item("Songs", "");
	
	filterItems[filterSize] = NULL;
	
	self->items = filterItems;

	return true;
}

bool initializeSearchUi() {
	searchUi->window = newwin(LINES - headerHeight, COLS, headerHeight, 0);
	if(searchUi->window)
		return true;
	return false;
}

static void freeFilter(struct Menu *self) {
	for(size_t i=0; self->items[i]; i++)
		free_item(self->items[i]);
	free(self->items);
	self->items = NULL;
}

static void handleKeypress(int key, char *sourceDir) {
	switch(key) {
		case KEY_DOWN:
			menu_driver(activeMenu->menu, REQ_DOWN_ITEM);
			break;
		case KEY_UP:
			menu_driver(activeMenu->menu, REQ_UP_ITEM);
			break;
		case KEY_PPAGE:
			menu_driver(activeMenu->menu, REQ_SCR_UPAGE);
			break;
		case KEY_NPAGE:
			menu_driver(activeMenu->menu, REQ_SCR_DPAGE);
			break;
		case KEY_LEFT:
			activeMenu = filters;
			break;
		case KEY_RIGHT:
			activeMenu = content;
			break;
		default:
			// all handleSelect functions should return true if the content menu should be focused.
			if(activeMenu->handleSelect(activeMenu, key, sourceDir))
				activeMenu = content;
			break;
		break;
	}
}

static bool filterHandleSelect(struct Menu *self, int key, char *sourceDir) {
	return true;
}
