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
static bool filterHandleSelect(struct Menu *self, int key, char *sourceDir);
static bool playlistsSetItems(struct Menu *self, char *sourceDir);
static void playlistsFreeItems(struct Menu *self);
static bool playlistsHandleSelect(struct Menu *self, int key, char *sourceDir);
static bool artistsSetItems(struct Menu *self, char *sourceDir);
static void artistsFreeItems(struct Menu *self);
static bool artistsHandleSelect(struct Menu *self, int key, char *sourceDir);
static bool episodesSetItems(struct Menu *self, char *sourceDir);
static void episodesFreeItems(struct Menu *self);
static bool episodesHandleSelect(struct Menu *self, int key, char *sourceDir);
static bool showsSetItems(struct Menu *self, char *sourceDir);
static void showsFreeItems(struct Menu *self);
static bool showsHandleSelect(struct Menu *self, int key, char *sourceDir);
static bool likedSongsSetItems(struct Menu *self, char *sourceDir);
static void likedSongsFreeItems(struct Menu *self);
static bool likedSongsHandleSelect(struct Menu *self, int key, char *sourceDir);
static bool albumsSetItems(struct Menu *self, char *sourceDir);
static void albumsFreeItems(struct Menu *self);
static bool albumsHandleSelect(struct Menu *self, int key, char *sourceDir);

static struct Menu _filters = {
	.menu = NULL,
	.items = NULL, 
	.setItems = &filterSetItems,
	.freeItems = &filterFreeItems,
	.handleSelect = &filterHandleSelect
};
struct Menu *filters = &_filters;

static struct Menu _playlists = {
	.menu = NULL,
	.items = NULL,
	.setItems = &playlistsSetItems,
	.freeItems = &playlistsFreeItems,
	.handleSelect = &playlistsHandleSelect,
};
static struct Menu *playlists = &_playlists;

static struct Menu _albums = {
	.menu = NULL,
	.items = NULL,
	.setItems = &albumsSetItems,
	.freeItems = &albumsFreeItems,
	.handleSelect = &albumsHandleSelect,
};
static struct Menu *albums = &_albums;

static struct Menu _artists = {
	.menu = NULL,
	.items = NULL,
	.setItems = &artistsSetItems,
	.freeItems = &artistsFreeItems,
	.handleSelect = &artistsHandleSelect,
};
static struct Menu *artists = &_artists;

static struct Menu _episodes = {
	.menu = NULL,
	.items = NULL,
	.setItems = &episodesSetItems,
	.freeItems = &episodesFreeItems,
	.handleSelect = &episodesHandleSelect,
};
static struct Menu *episodes = &_episodes;

static struct Menu _shows = {
	.menu = NULL,
	.items = NULL,
	.setItems = &showsSetItems,
	.freeItems = &showsFreeItems,
	.handleSelect = &showsHandleSelect,
};
static struct Menu *shows = &_shows;

static struct Menu _likedSongs = {
	.menu = NULL,
	.items = NULL,
	.setItems = &likedSongsSetItems,
	.freeItems = &likedSongsFreeItems,
	.handleSelect = &likedSongsHandleSelect,
};
static struct Menu *likedSongs = &_likedSongs;

static struct Menu *content;
static struct Menu *activeMenu;

bool initializeLibraryWin(char *sourceDir) {
	if(albumsIdx == defaultIdx)
		content = albums;
	else if (artistsIdx == defaultIdx)
		content = artists;
	else if (playlistsIdx == defaultIdx)
		content = playlists;
	else if (showsIdx == defaultIdx)
		content = playlists;
	else if (episodesIdx == defaultIdx)
		content = episodes;
	else if (episodesIdx == defaultIdx)
		content = likedSongs;
	else
		return false;

	libraryWin->display = &display;
	libraryWin->close = &close;
	libraryWin->handleKeypress = &handleKeypress;

	if(!filters->setItems(filters, NULL))
		return false;

	MENU *filterMENU = assembleMenu(filters->items, libraryWin->window, 0, 0, "", true);
	if(filterMENU == NULL) {
		filters->freeItems(filters);
		return false;
	}

	filters->menu = filterMENU;
	activeMenu = filters;
	
	MENU *contentMENU = assembleMenu(content->items, libraryWin->window, 0, 0, "", false);
	if(contentMENU == NULL) {
		filters->freeItems(filters);
		free_menu(filterMENU);
		return false;
	}

	playlists->menu = contentMENU;
	albums->menu = contentMENU;
	artists->menu = contentMENU;
	shows->menu = contentMENU;
	episodes->menu = contentMENU;
	likedSongs->menu = contentMENU;
	
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

static bool filterHandleSelect(struct Menu *self, int key, char *sourceDir) {
	return true;
}

static bool playlistsSetItems(struct Menu *self, char *sourceDir) {
	return true;
}

static void playlistsFreeItems(struct Menu *self) {
	
}

static bool playlistsHandleSelect(struct Menu *self, int key, char *sourceDir) {
	return true;
}

static bool artistsSetItems(struct Menu *self, char *sourceDir) {
	return true;
}

static void artistsFreeItems(struct Menu *self) {

}

static bool artistsHandleSelect(struct Menu *self, int key, char *sourceDir) {
	return true;
}

static bool episodesSetItems(struct Menu *self, char *sourceDir) {
	return true;
}

static void episodesFreeItems(struct Menu *self) {

}

static bool episodesHandleSelect(struct Menu *self, int key, char *sourceDir) {
	return true;
}

static bool showsSetItems(struct Menu *self, char *sourceDir) {
	return true;
}

static void showsFreeItems(struct Menu *self) {

}

static bool showsHandleSelect(struct Menu *self, int key, char *sourceDir) {
	return true;
}

static bool likedSongsSetItems(struct Menu *self, char *sourceDir) {
	return true;
}

static void likedSongsFreeItems(struct Menu *self) {

}

static bool likedSongsHandleSelect(struct Menu *self, int key, char *sourceDir) {
	return true;
}

static bool albumsSetItems(struct Menu *self, char *sourceDir) {
	return true;
}

static void albumsFreeItems(struct Menu *self) {

}

static bool albumsHandleSelect(struct Menu *self, int key, char *sourceDir) {
	return true;
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
			menu_driver(activeMenu->menu, REQ_DOWN_ITEM);
			break;
		case KEY_UP:
			menu_driver(activeMenu->menu, REQ_UP_ITEM);
			break;
		case KEY_LEFT:
			activeMenu = filters;
			break;
		case KEY_RIGHT:
			activeMenu = content;
			break;
		case 10:
			activeMenu->handleSelect(activeMenu, key, sourceDir);
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


