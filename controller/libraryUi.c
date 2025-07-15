#include <stdlib.h>
#include <stdbool.h>
#include <ncurses.h>
#include <menu.h>

#include "config.h"
#include "ui.h"

static unsigned int libraryFilterMenuWidth = 0;
void freeLibraryFilterItems(ITEM **filters);
bool filterSetItems(struct Menu *self, char *null);
void filterFreeItems(struct Menu *self);

bool filterSetItems(struct Menu *self, char *null) {
	self->items[albumsIdx] = new_item("Albums","");
	self->items[artistsIdx] = new_item("Artists", "");
	self->items[playlistsIdx] = new_item("Playlists","");
	self->items[showsIdx] = new_item("Shows", "");
	self->items[episodesIdx] = new_item("Episodes", "");
	self->items[likedSongsIdx] = new_item("Liked Songs", "");
	return true;
}

void filterFreeItems(struct Menu *self) {
	freeLibraryFilterItems(self->items);
}

bool drawLibraryWin(char *sourceDir) {
	unsigned int filterSize = 6;
	ITEM *filterItems[filterSize + 1];
	struct Menu filters = {filterItems, &filterSetItems, &filterFreeItems};
	filters.setItems(&filters, NULL);
	filters.items[filterSize] = NULL;

	MENU *filterMenu = assembleMenu(filters.items, libraryWin, 0, 0, "", true);
	wrefresh(libraryWin);

	int ch;
	ITEM *selection;
	unsigned int selectedIdx = 0;
	bool running = true;
	while(running && (ch = wgetch(libraryWin))) {
		switch(ch) {
			case KEY_DOWN:
				menu_driver(filterMenu, REQ_DOWN_ITEM);
				break;
			case KEY_UP:
				menu_driver(filterMenu, REQ_UP_ITEM);
				break;
			case 10:
				selection = current_item(filterMenu);
				selectedIdx = item_index(selection);
				break;
			case 'q':
				running = false;
			break;
		}
		wrefresh(libraryWin);
	}
	unpost_menu(filterMenu);
	free_menu(filterMenu);
	filters.freeItems(&filters);

	return true;
}

void freeLibraryFilterItems(ITEM **filters) {
	unsigned int i = 0;
	ITEM *thisFilter;
	while((thisFilter = filters[i]) != NULL) {
		free(thisFilter);
		i++;
	}
}


