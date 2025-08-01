#include <stdlib.h>
#include <stdbool.h>
#include <ncurses.h>
#include <menu.h>
#include <string.h>
#include "config.h"
#include "ui.h"
#include "spotifyCommands.h"
#include "utils.h"

static const size_t filterSize = 6;
static unsigned int libraryFilterMenuWidth = 0;
static void freeLibraryFilterItems(ITEM **filters);
static struct Menu **filterMenusOrdered; 

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
static bool handleLazyTrackSelect(struct Menu *self, int key, char *sourceDir);

static ITEM **allocStaticLibraryItems(char *sourceDir, FILE* (*func)(char *, int, int)); 
static void freeAllocatedItemArr(ITEM **items); 
static ITEM **makeArtistItems(FILE *artistsNewLineList);
bool isEpisode = false;

static struct LazyTracker *likedSongsTracker;

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

static struct Menu _lazyArtistAlbums = {
	.menu = NULL,
	.items = NULL,
	.setItems = NULL,
	.freeItems = NULL,
	.handleSelect = &albumsHandleSelect,
};
static struct Menu *lazyArtistAlbums = &_lazyArtistAlbums;

static struct Menu _lazyTracks = {
	.menu = NULL,
	.items = NULL,
	.setItems = NULL,
	.freeItems = NULL,
	.handleSelect = &handleLazyTrackSelect,
};
static struct Menu *lazyTracks = &_lazyTracks;

static struct Menu *content;
static struct Menu *activeMenu;

bool initializeLibraryWin(char *sourceDir) {
	filterMenusOrdered = malloc(sizeof(struct Menu *) * (filterSize + 1));
	filterMenusOrdered[albumsIdx] = albums;
	filterMenusOrdered[artistsIdx] = artists;
	filterMenusOrdered[playlistsIdx] = playlists;
	filterMenusOrdered[showsIdx] = shows;
	filterMenusOrdered[episodesIdx] = episodes;
	filterMenusOrdered[likedSongsIdx] = likedSongs;

	if(defaultIdx < filterSize)
		content = filterMenusOrdered[defaultIdx];
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
	
	for(size_t i = 0; i < filterSize; i++) {
		if(!filterMenusOrdered[i]->setItems(filterMenusOrdered[i], sourceDir))
			return false;
	}

	MENU *contentMENU = assembleMenu(content->items, libraryWin->window, 0, 12, "", false);
	if(contentMENU == NULL) {
		filters->freeItems(filters);
		free_menu(filterMENU);
		return false;
	}

	for(size_t i = 0; i < filterSize; i++) {
		filterMenusOrdered[i]->menu = contentMENU;
	}
	lazyArtistAlbums->menu = contentMENU;
	lazyTracks->menu = contentMENU;

	return true;
}

static bool filterSetItems(struct Menu *self, char *null) {
	ITEM **filterItems = malloc(sizeof(ITEM *) * (filterSize + 1));
	if(filterItems == NULL)
		return false;

	filterItems[albumsIdx] = new_item("Albums","");
	filterItems[artistsIdx] = new_item("Artists", "");
	filterItems[playlistsIdx] = new_item("Playlists","");
	filterItems[showsIdx] = new_item("Shows", "");
	filterItems[episodesIdx] = new_item("Episodes", "");
	filterItems[likedSongsIdx] = new_item("Liked Songs", "");

	filterItems[filterSize] = NULL;

	bool allocFailed = false;
	for(unsigned int i = 0; i < filterSize; i++) {
		if(filterItems[i] == NULL)
			allocFailed = true;
		break;
	}
	if(allocFailed) {
		for(unsigned int i = 0; i < filterSize; i++) {
			if(filterItems[i] != NULL)
				free_item(filterItems[i]);
		}
		return false;
	}

	self->items = filterItems;
	return true;
}

static void filterFreeItems(struct Menu *self) {
	freeLibraryFilterItems(self->items);
	free(self->items);
	self->items = NULL;
}

static bool filterHandleSelect(struct Menu *self, int key, char *sourceDir) {
	if(key != 10)
		return false;

	size_t selectedIdx = item_index(current_item(self->menu));
	unpost_menu(content->menu);
	set_menu_items(content->menu, NULL);
	if(currentLazyTracker) {
		currentLazyTracker->clean(currentLazyTracker);
		currentLazyTracker = NULL;
	}
	if(backLazyTracker) {
		backLazyTracker->clean(backLazyTracker);
		backLazyTracker = NULL;
		backLazy = NULL;
	}
	content = filterMenusOrdered[selectedIdx];
	set_menu_items(content->menu, content->items);
	post_menu(content->menu);

	return true;
}

static bool playlistsSetItems(struct Menu *self, char *sourceDir) {
	self->items = allocStaticLibraryItems(sourceDir, &getPlaylistsNewLineList);
	if(self->items)
		return true;

	return false;
}

static void playlistsFreeItems(struct Menu *self) {
	freeAllocatedItemArr(self->items);	
}

static bool playlistsHandleSelect(struct Menu *self, int key, char *sourceDir) {
	ITEM *selection = current_item(self->menu);
	char *uri = item_userptr(selection);
	if(key == 10) {
		playContext(uri);
	}
	else if (key == 'i') {
		char playlistId[100];
		uriToId(playlistId, uri);
		FILE *playlistTracksNewLineList = getPlaylistTracksNewLineList(playlistId, 50,sourceDir); 
		if(!playlistTracksNewLineList)
			return false;

		if(setCurrentLazy(playlistTracksNewLineList, &initLazyTracker, &lazyLoadTracks, &cleanLazyLoadedTracks)) {
			lazyContext = uri;
			unpost_menu(content->menu);
			set_menu_items(content->menu, NULL);
			content = lazyTracks;
			content->items = currentLazyTracker->tracks;
			set_menu_items(content->menu, content->items);
			post_menu(content->menu);
			return true;

		}
		return true;

		
	}
	else if (key == '\'') {
		int status = playContextAt(item_userptr(self->items[item_index(current_item(self->menu))]), 0);
		if(status == 0)
			shuffleOff();
	}
	return false;
}

static bool artistsSetItems(struct Menu *self, char *sourceDir) {
	FILE *artistsNewLineList = getArtistsNewLineList(sourceDir);
	if(!artistsNewLineList)
		return false;

	self->items = makeArtistItems(artistsNewLineList);
	if(self->items)
		return true;

	return false;
}

static void artistsFreeItems(struct Menu *self) {

}

static bool artistsHandleSelect(struct Menu *self, int key, char *sourceDir) {
	ITEM *selection = current_item(self->menu);
	char *uri = item_userptr(selection);
	if(key == 10) {
		playContext(uri);	
	}
	else if (key == 'i') {
		char artistId[100];
		uriToId(artistId, uri);
		FILE *artistAlbumsNewLineList = getArtistAlbumsNewLineList(artistId, 50, sourceDir);
		if(artistAlbumsNewLineList && setCurrentLazy(artistAlbumsNewLineList, &initLazyTracker, &lazyLoadTracks, &cleanLazyLoadedTracks)) {
			unpost_menu(content->menu);
			set_menu_items(content->menu, NULL);
			content = lazyArtistAlbums;
			content->items = currentLazyTracker->tracks;
			set_menu_items(content->menu, content->items);
			post_menu(content->menu);
			return true;
		}
	}
	return false;
}

static bool episodesSetItems(struct Menu *self, char *sourceDir) {
	self->items = allocStaticLibraryItems(sourceDir, &getEpisodesNewLineList);
	if(self->items)
		return true;

	return false;
}

static void episodesFreeItems(struct Menu *self) {
	freeAllocatedItemArr(self->items);	
}

static bool episodesHandleSelect(struct Menu *self, int key, char *sourceDir) {
	ITEM *selectedItem = current_item(content->menu);
	size_t leftOffIndex = item_index(selectedItem);
	char *uri = item_userptr(selectedItem);
	bool isExpandOption = strcmp(item_description(selectedItem), ".") == 0;
	if(key == 10) {
		if(isExpandOption) {
			unpost_menu(content->menu);
	    		set_menu_items(content->menu, NULL);
			currentLazyTracker->expand(currentLazyTracker, sourceDir);
		    	content->items = currentLazyTracker->tracks;
	    		set_menu_items(content->menu, content->items);
	    		post_menu(content->menu);
	    		set_current_item(content->menu, content->items[leftOffIndex]);
			return true;
		} 
		else 
			playTrack(uri);	
	}
	else if(isExpandOption) {
		return false;
	}
	return true;
}

static bool showsSetItems(struct Menu *self, char *sourceDir) {
	self->items = allocStaticLibraryItems(sourceDir, &getShowsNewLineList);
	if(self->items)
		return true;

	return false;
}

static void showsFreeItems(struct Menu *self) {
	freeAllocatedItemArr(self->items);	
}

static bool showsHandleSelect(struct Menu *self, int key, char *sourceDir) {
	ITEM *selection = current_item(self->menu);
	char *uri = item_userptr(selection);
	if(key == 10 || key == '\'')
		playContext(uri);
	else if(key == 'i') {
		char showId[100];
		uriToId(showId, uri);
		FILE *showEpisodesNewLineList = getShowEpisodesNewLineList(showId, 50,sourceDir); 
		if(!showEpisodesNewLineList)
			return false;

		if(setCurrentLazy(showEpisodesNewLineList, &initLazyTracker, &lazyLoadTracks, &cleanLazyLoadedTracks)) {
			lazyContext = uri;
			unpost_menu(content->menu);
			set_menu_items(content->menu, NULL);
			content = lazyTracks;
			content->items = currentLazyTracker->tracks;
			set_menu_items(content->menu, content->items);
			post_menu(content->menu);
			isEpisode = true;
			return true;

		}
		return true;
	} 
	return true;
}

static bool likedSongsSetItems(struct Menu *self, char *sourceDir) {
	FILE *likedNewLineList = getLikedSongsNewLineList(sourceDir, 50);
	if(!likedNewLineList)
		return false;

	likedSongsTracker = initLazyTracker(likedNewLineList, 50, &lazyLoadTracks, &cleanLazyLoadedTracks);
	if(!likedSongsTracker)
		return false;

	self->items = likedSongsTracker->tracks;

	return true;
}

static void likedSongsFreeItems(struct Menu *self) {

}

static bool likedSongsHandleSelect(struct Menu *self, int key, char *sourceDir) {
	if (key == 10) {
        	ITEM *selectedItem = current_item(content->menu);
		size_t leftOffIndex = item_index(selectedItem);
        	if (strcmp(item_description(selectedItem), ".") == 0) {
			unpost_menu(content->menu);
	    		set_menu_items(content->menu, NULL);
            		likedSongsTracker->expand(likedSongsTracker, sourceDir);

		    	content->items = likedSongsTracker->tracks;
	    		set_menu_items(content->menu, content->items);
	    		post_menu(content->menu);
	    		set_current_item(content->menu, content->items[leftOffIndex]);
            		return true;
        	} 
		else {
			playTrack(item_userptr(selectedItem));
		}
	}
    	return true;
}

static bool albumsSetItems(struct Menu *self, char *sourceDir) {
	self->items = allocStaticLibraryItems(sourceDir, &getAlbumsNewLineList);
	if(self->items) {
		return true;
	}

	return false;
}

static void albumsFreeItems(struct Menu *self) {
	freeAllocatedItemArr(self->items);	
}

static bool albumsHandleSelect(struct Menu *self, int key, char *sourceDir) {
        ITEM *selectedItem = current_item(content->menu);
	size_t leftOffIndex = item_index(selectedItem);
	char *userptr = item_userptr(selectedItem);
	bool isExpandOption = strcmp(item_description(selectedItem), ".") == 0;
	if(key == 10) {
		if(isExpandOption) {
			unpost_menu(content->menu);
	    		set_menu_items(content->menu, NULL);
			currentLazyTracker->expand(currentLazyTracker, sourceDir);
		    	content->items = currentLazyTracker->tracks;
	    		set_menu_items(content->menu, content->items);
	    		post_menu(content->menu);
	    		set_current_item(content->menu, content->items[leftOffIndex]);
			return true;
		} 
		else 
			playContext(item_userptr(self->items[item_index(current_item(self->menu))]));	
	}
	else if(isExpandOption) {
		return false;
	}
	else if(key == 'i') {
		char albumId[100];
		uriToId(albumId, userptr);
		FILE *albumTracksNewLineList = getAlbumTracksNewLineList(albumId, sourceDir); 
		if(!albumTracksNewLineList)
			return false;

		backLazyTracker = currentLazyTracker;
		backLazy = content;
		backLazyContext = lazyContext;

		if(setCurrentLazy(albumTracksNewLineList, &initLazyTracker, &lazyLoadTracks, &cleanLazyLoadedTracks)) {
			lazyContext = userptr;
			unpost_menu(content->menu);
			set_menu_items(content->menu, NULL);
			content = lazyTracks;
			content->items = currentLazyTracker->tracks;
			set_menu_items(content->menu, content->items);
			post_menu(content->menu);
			return true;

		}
		return true;
	}
	else if (key == '\'') {
		int status = playContextAt(item_userptr(self->items[item_index(current_item(self->menu))]), 0);
		if(status == 0)
			shuffleOff();
	}


	return false;
}

static bool handleLazyTrackSelect(struct Menu *self, int key, char *sourceDir) {
	ITEM *track = current_item(self->menu);
	size_t leftOffIndex = item_index(track);
	bool isExpandOption = strcmp(item_description(track), ".") == 0;
	if(key == 10) {
		if(isExpandOption) {
			unpost_menu(content->menu);
	    		set_menu_items(content->menu, NULL);
			currentLazyTracker->expand(currentLazyTracker, sourceDir);
		    	content->items = currentLazyTracker->tracks;
	    		set_menu_items(content->menu, content->items);
	    		post_menu(content->menu);
	    		set_current_item(content->menu, content->items[leftOffIndex]);
			return true;
		}
		if(isEpisode)
			playTrack(item_userptr(track));
		else
			playContextAt(lazyContext, item_index(track));
	}
	else if(isExpandOption)
		return false;
	return false;
}

static bool display(char *sourceDir) {
	post_menu(filters->menu);
	post_menu(content->menu);
	return true;
}

static void close() {
	unpost_menu(filters->menu);
	unpost_menu(content->menu);
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

static void freeLibraryFilterItems(ITEM **filters) {
	unsigned int i = 0;
	ITEM *thisFilter;
	while((thisFilter = filters[i]) != NULL) {
		free(thisFilter);
		i++;
	}
}

static ITEM **allocStaticLibraryItems(char *sourceDir, FILE* (*func)(char *, int, int)) {
	int limit = 50;
	int offset = 0;
	FILE *newLineList = func(sourceDir, limit, offset);
	if(newLineList == NULL)
		return NULL;

	char *line = NULL;
	size_t len = 0;
	if(getline(&line, &len, newLineList) == -1) {
		fclose(newLineList);
		return NULL;
	}
	line[strcspn(line, "\n")] = '\0';

	int size = atoi(line);
	ITEM **items = malloc(sizeof(ITEM *) * (size + 1));
	if(!items) {
		free(line);
		fclose(newLineList);
		return NULL;
	}
	items[size] = NULL;

	int itemIdx = 0;
	size_t itemLine = 1;
	bool isCountLine = false;
	char *itemName;
	char *itemDesc;
	char *itemUserPtr;
	while(itemIdx < size) {
		if(getline(&line, &len, newLineList) == -1) {
			// request next set of items
			offset+=limit;
			fclose(newLineList);
			newLineList = func(sourceDir, limit, offset);
			if(newLineList == NULL) {
				free(line);
				freeAllocatedItemArr(items);
				return NULL;
			}
			itemLine = 1;
			isCountLine = true;
			continue;
		}
		line[strcspn(line, "\n")] = '\0';
		// temporary fix for wide (japanese, ect.) characters breaking menu loading
		// lets be real this is probably permenent lol
		for (char *p = line; *p; ++p) {
    			if ((unsigned char)*p >= 128) *p = '?';
		}
		if(isCountLine) {
			isCountLine = false;
			continue;
		}

		if(itemLine == 1) {
			itemName = strdup(line);
			if(itemName == NULL) {
				fclose(newLineList);
				freeAllocatedItemArr(items);
				free(line);
				return NULL;
			}
		}
		else if (itemLine == 2) {
			itemDesc = strdup(line);
			if(itemDesc == NULL) {
				fclose(newLineList);
				freeAllocatedItemArr(items);
				free(itemName);
				free(line);
				return NULL;
			}
		}
		else {
			items[itemIdx] = new_item(itemName, itemDesc);
			itemUserPtr = strdup(line);
			if (itemUserPtr == NULL) {
				freeAllocatedItemArr(items);
				fclose(newLineList);
				free(itemName);
				free(itemDesc);
				free(line);
				return NULL;
			}
			set_item_userptr(items[itemIdx], itemUserPtr);
			itemIdx++;
		}

		itemLine = (itemLine + 1) % 3;
	}
	return items;
}

static void freeAllocatedItemArr(ITEM **items) {
	size_t i = 0;
	ITEM *thisItem;
	while((thisItem = items[i]) != NULL) {
		free((char *)item_name(thisItem));
		free((char *)item_description(thisItem));
		free(item_userptr(thisItem));
		free_item(thisItem);
		i++;
	}
	free(items);
}

static ITEM **makeArtistItems(FILE *artistsNewLineList) {
	char *line = NULL;
	size_t len = 0;
	if(getline(&line, &len, artistsNewLineList) == -1) {
		fclose(artistsNewLineList);
		return NULL;
	}
	line[strcspn(line, "\n")] = '\0';

	int size = atoi(line);
	ITEM **items = malloc(sizeof(ITEM *) * (size + 1));
	if(!items) {
		free(line);
		fclose(artistsNewLineList);
		return NULL;
	}
	items[size] = NULL;
	
	int itemIdx = 0;
	size_t nLine = 1;
	char *itemName;
	char *itemUserPtr;
		
	while (getline(&line, &len, artistsNewLineList) != -1) {
		line[strcspn(line, "\n")] = '\0';
		for (char *p = line; *p; ++p) {
    			if ((unsigned char)*p >= 128) *p = '?';
		}

		if(nLine % 2 != 0) {
			itemName = strdup(line);
			if(itemName == NULL) {
				fclose(artistsNewLineList);
				freeAllocatedItemArr(items);
				free(line);
				return NULL;
			}
		}
		else {
			items[itemIdx] = new_item(itemName, "");
			itemUserPtr = strdup(line);
			if (itemUserPtr == NULL) {
				freeAllocatedItemArr(items);
				fclose(artistsNewLineList);
				free(itemName);
				free(line);
				return NULL;
			}
			set_item_userptr(items[itemIdx], itemUserPtr);
			itemIdx++;
		}
		nLine++;

	}
	return items;


}


