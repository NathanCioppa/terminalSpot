#include <ncurses.h>
#include <menu.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "ui.h"
#include "config.h"
#include "spotifyCommands.h"

static bool display(char *sourceDir);
static void close();
static void handleKeypress(int key, char *sourceDir);
static void handleKeypress(int key, char *sourceDir); 
static bool handleSearch(char *sourceDir); 
static void displaySearchInput(); 
static bool validateSearch(char *filterBuf, size_t filterBufSize, char *queryBuf, size_t queryBufSize, char *fullSearchBuf); 
static void freeSearchItems(struct Menu *self); 
static bool displaySearchResults(char *filter, FILE *searchResultsNewLineList);
static bool handleSearchResultSelect(struct Menu *content, int key, char *sourceDir); 
static void clearSearchDisplay(unsigned int startRow);

static void handleAlbumSelect(ITEM *selection, int key, char *sourceDir);
static void handlePlaylistSelect(ITEM *selection, int key, char *sourceDir);
static void handleTrackSelect(ITEM *selection, int key, char *sourceDir);
static void handleSongSelect(ITEM *selection, int key, char *sourceDir);
static void handleArtistSelect(ITEM *selection, int key, char *sourceDir);
static void handleEpisodeSelect(ITEM *selection, int key, char *sourceDir);
static void handleShowSelect(ITEM *selection, int key, char *sourceDir);
int defaultPlayFromUri(char *uri);
void extractUriType(char *uriTypeBuf, char *uri); 

static bool searchRow = 0;
static bool searchCol = 0;
static char *searchLabel = "Search:";
static size_t searchFilterSize = 2;
static size_t maxSearchQuerySize = 100;


struct FilterAction {
	char *filter;
	void (*action)(ITEM *selection, int key, char *sourceDir);
};

static struct Window _searchUi = {
	.window = NULL,
	.display = &display,
	.close = &close,
	.handleKeypress = &handleKeypress,
};
struct Window *searchUi = &_searchUi;

static struct Menu *content = NULL;

static struct LazyTracker *currentLazy = NULL;

static struct FilterAction filterActionAlbums   = { "al", &handleAlbumSelect };
static struct FilterAction filterActionPlaylists = { "pl", &handlePlaylistSelect };
static struct FilterAction filterActionTracks    = { "tr", &handleTrackSelect };
static struct FilterAction filterActionSongs     = { "so", &handleSongSelect };
static struct FilterAction filterActionArtists   = { "ar", &handleArtistSelect };
static struct FilterAction filterActionEpisodes  = { "ep", &handleEpisodeSelect };
static struct FilterAction filterActionShows     = { "sh", &handleShowSelect };

static struct FilterAction *allFilterActions[] = {
    &filterActionAlbums,
    &filterActionPlaylists,
    &filterActionTracks,
    &filterActionSongs,
    &filterActionArtists,
    &filterActionEpisodes,
    &filterActionShows,
    NULL, 
};

static struct FilterAction *activeFilterAction = NULL;


static bool display(char *sourceDir) {
	struct Menu *searchResults = malloc(sizeof(struct Menu));
	if(!searchResults)
		return false;

	searchResults->menu = NULL;
	searchResults->items = NULL;
	searchResults->setItems = NULL;
	searchResults->freeItems = &freeSearchItems;
	searchResults->handleSelect = &handleSearchResultSelect;

	content = searchResults;
	return true;
}

static void close() {
	if(content) {
		content->freeItems(content);
		free_menu(content->menu);
		content = NULL;
	}

	if(currentLazy) {
		free(currentLazy);
		currentLazy = NULL;
	}
}

bool initializeSearchUi() {
	searchUi->window = newwin(LINES - headerHeight, COLS, headerHeight, 0);
	if(searchUi->window)
		return true;
	return false;
}

static void freeSearchItems(struct Menu *self) {
	if(!self->items)
		return;
	ITEM *item = NULL;
	for(size_t i=0; (item = self->items[i]); i++) {
		if(strcmp(item_description(item), ".") != 0) {
			free((char *)item_name(item));
			free((char *)item_description(item));
			free(item_userptr(item));
		}
		free_item(item);
		item = NULL;
	}
	free(self->items);
}

static bool handleSearchResultSelect(struct Menu *content, int key, char *sourceDir) {
	ITEM *selection = current_item(content->menu);
	bool isExpandOption = strcmp(item_description(selection), ".") == 0;
	if(key == 10) {
		if(isExpandOption) {
			size_t leftOffIndex = item_index(selection);
			unpost_menu(content->menu);
			set_menu_items(content->menu, NULL);
			currentLazy->expand(currentLazy, sourceDir, true);
			
			content->items = currentLazy->tracks;
			set_menu_items(content->menu, content->items);
			post_menu(content->menu);
			set_current_item(content->menu, content->items[leftOffIndex]);
			return true;
		}
		char *uri = item_userptr(selection);
		defaultPlayFromUri(uri);
	} 
	else 
		activeFilterAction->action(selection, key, sourceDir);
	return true;
}

static void handleKeypress(int key, char *sourceDir) {
	switch(key) {
		case 's':
			if(handleSearch(sourceDir))
				wrefresh(searchUi->window);
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

static bool handleSearch(char *sourceDir) {
	char searchBuf[256];
	displaySearchInput();
	echo();
	wgetnstr(searchUi->window, searchBuf, sizeof(searchBuf) - 1);
	mvprintw(searchRow, searchCol + strlen(searchLabel),"%s", searchBuf);
	noecho();
	char filter[searchFilterSize+1];
	char query[maxSearchQuerySize+1];
	if(!validateSearch(filter, searchFilterSize+1, query, maxSearchQuerySize+1, searchBuf)) 
		return false;

	FILE *searchResults = getSearchResults(query, filter, 10, sourceDir);
	if(!searchResults) 
		return false;

	struct FilterAction *fa = NULL;
	for(size_t i=0; (fa=allFilterActions[i]); i++) {
		if(strcmp(fa->filter, filter) == 0) {
			activeFilterAction = fa;
			break;
		}
	}
	return displaySearchResults(filter, searchResults);
}

static void displaySearchInput() {
	int width = getmaxx(searchUi->window);
	mvwprintw(searchUi->window, searchRow, 0, "%*s", width, "");
	mvwprintw(searchUi->window, searchRow, searchCol, "%s", searchLabel);
}

static bool validateSearch(char *filterBuf, size_t filterBufSize, char *queryBuf, size_t queryBufSize, char *fullSearchBuf) {
	bool isInvalid = fullSearchBuf[0] == '\0' || strlen(fullSearchBuf) <= filterBufSize;
	if(isInvalid)
		return false;

	filterBuf[filterBufSize-1] = '\0';
	for(size_t i=0; i<filterBufSize-1; i++) 
		filterBuf[i] = fullSearchBuf[i];

	queryBuf[queryBufSize - 1] = '\0';
	for(size_t i=0; (i<queryBufSize-1)&& fullSearchBuf[i+filterBufSize]; i++)
		queryBuf[i] = fullSearchBuf[filterBufSize + i];

	return true;
}

//struct LazyTracker *initLazyTracker(FILE *newLineList, int limitPerRequest, bool (*expand)(struct LazyTracker *self, char *sourceDir), void (*clean)(struct LazyTracker *self)) { 
static bool displaySearchResults(char *filter, FILE *searchResultsNewLineList) {
	struct LazyTracker *newLazy = initLazyTracker(searchResultsNewLineList, 10, &lazyLoadTracks, &cleanLazyLoadedTracks);
	if(!newLazy)
		return false;

	if(!currentLazy) {
		MENU *resultsMENU = assembleMenu(newLazy->tracks, searchUi->window, 1, 0, "", false);
		if(!resultsMENU) {
			newLazy->clean(newLazy);
			free(newLazy);
			return false;
		}
		content->menu = resultsMENU;
		content->items = newLazy->tracks;
		currentLazy = newLazy;
		post_menu(content->menu);

		return true;
	}

	unpost_menu(content->menu);
	set_menu_items(content->menu, NULL);
	currentLazy->clean(currentLazy);
	content->items = newLazy->tracks;
	currentLazy = newLazy;
	set_menu_items(content->menu, content->items);
	post_menu(content->menu);
	return true;
}

static void handleAlbumSelect(ITEM *selection, int key, char *sourceDir) {

}

static void handlePlaylistSelect(ITEM *selection, int key, char *sourceDir) {
    // TODO: Implement playlist selection logic
}

static void handleTrackSelect(ITEM *selection, int key, char *sourceDir) {
    // TODO: Implement track selection logic
}

static void handleSongSelect(ITEM *selection, int key, char *sourceDir) {
    // TODO: Implement song selection logic
}

static void handleArtistSelect(ITEM *selection, int key, char *sourceDir) {
    // TODO: Implement artist selection logic
}

static void handleEpisodeSelect(ITEM *selection, int key, char *sourceDir) {
    // TODO: Implement episode selection logic
}

static void handleShowSelect(ITEM *selection, int key, char *sourceDir) {
    // TODO: Implement show selection logic
}


int defaultPlayFromUri(char *uri) {
	char uriType[32] = "\0";
	extractUriType(uriType, uri);
	if(uriType[0] == '\0')
		return -2;

	char *playableContexts[] = {"album","show","playlist","artist", NULL};
	char *playIndividual[] =  {"track", "episode",NULL};

	char *checkType;
	for(size_t i=0; (checkType = playableContexts[i]); i++) {
		if(strcmp(checkType, uriType) == 0)
			return playContext(uri);	
	}
	for(size_t i=0; (checkType = playIndividual[i]); i++) {
		if(strcmp(checkType, uriType) == 0)
			return playTrack(uri);
	}
	return -1;
}

void extractUriType(char *uriTypeBuf, char *uri) {
	bool isReadingType = false;
	size_t uriTypeStart = 0;
	for(size_t i=0; uri[i]; i++) {
		if(!isReadingType && uri[i] == ':') {
			isReadingType=true;
			uriTypeStart = i+1;
		} 
		else if (isReadingType) {
			if (uri[i] == ':') {
				uriTypeBuf[i - uriTypeStart] = '\0';
				return;
			} 
			else {
				uriTypeBuf[i - uriTypeStart] = uri[i];
			}
		}
	}
	uriTypeBuf = "\0";
}

