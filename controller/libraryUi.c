#include <stdlib.h>
#include <stdbool.h>
#include <ncurses.h>
#include <menu.h>

#include "config.h"
#include "ui.h"

static unsigned int libraryFilterMenuWidth = 0;
static void freeLibraryFilterItems(ITEM **filters);

static bool display(char *sourceDir);
static void close();
static void handleKeypress(int key, char *sourceDir);
static bool filterSetItems(struct Menu *self, char *null);
static void filterFreeItems(struct Menu *self);

static struct Menu _filters = {
	.menu = NULL,
	.items = NULL, 
	.setItems = &filterSetItems,
	.freeItems = &filterFreeItems,
};
static struct Menu *filters = &_filters;

bool initializeLibraryWin(char *sourceDir) {
	libraryWin->display = &display;
	libraryWin->close = &close;
	libraryWin->handleKeypress = &handleKeypress;

	if(!filters->setItems(filters, NULL))
		return false;

	MENU *filterMenu = assembleMenu(filters->items, libraryWin->window, 0, 0, "", true);
	if(filterMenu == NULL) {
		filters->freeItems(filters);
		return false;
	}

	filters->menu = filterMenu;
	return true;
}

static bool filterSetItems(struct Menu *self, char *null) {
	ITEM **filterItems = malloc(sizeof(ITEM *) * 7);
	if(filterItems == NULL)
		return false;
	self->items = filterItems;

	self->items[albumsIdx] = new_item("Albums","");
	self->items[artistsIdx] = new_item("Artists", "");
	self->items[playlistsIdx] = new_item("Playlists","");
	self->items[showsIdx] = new_item("Shows", "");
	self->items[episodesIdx] = new_item("Episodes", "");
	self->items[likedSongsIdx] = new_item("Liked Songs", "");

	self->items[6] = NULL;
	return true;
}

static void filterFreeItems(struct Menu *self) {
	freeLibraryFilterItems(self->items);
	free(self->items);
	self->items = NULL;
}

static bool display(char *sourceDir) {
	post_menu(filters->menu);
	return true;
}

static void close() {
	unpost_menu(filters->menu);
}

static void handleKeypress(int key, char *sourceDir) {
	switch(key) {
		case KEY_DOWN:
			menu_driver(filters->menu, REQ_DOWN_ITEM);
			break;
		case KEY_UP:
			menu_driver(filters->menu, REQ_UP_ITEM);
			break;
		case 10:

			break;
		break;
	}
}

static void freeLibraryFilterItems(ITEM **filters) {
	unsigned int i = 0;
	ITEM *thisFilter;
	while((thisFilter = filters[i]) != NULL) {
		free(thisFilter);
		i++;
	}
}


