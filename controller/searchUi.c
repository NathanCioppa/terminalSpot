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
static bool handleSearchResultSelect(); 
static void clearSearchDisplay(unsigned int startRow); 

//bool searchInputFocused = false;
static bool searchRow = 0;
static bool searchCol = 0;
static char *searchLabel = "Search:";
static size_t searchFilterSize = 2;
static size_t maxSearchQuerySize = 100;

static struct Window _searchUi = {
	.window = NULL,
	.display = &display,
	.close = &close,
	.handleKeypress = &handleKeypress,
};
struct Window *searchUi = &_searchUi;

static struct Menu *content = NULL;

static struct LazyTracker *currentLazy = NULL;

static char *filterAlbums = "al";
static char *filterPlaylists = "pl";
static char *filterTracks = "tr";
static char *filterSongs = "so";
static char *filterArtists = "ar";
static char *filterEpisodes = "ep";
static char *filterShows = "sh";

static bool display(char *sourceDir) {
	struct Menu *searchResults = malloc(sizeof(struct Menu));
	if(!searchResults)
		return false;

	searchResults->menu = NULL;
	searchResults->items = NULL;
	searchResults->setItems = NULL;//&setSearchItems;
	searchResults->freeItems = &freeSearchItems;
	searchResults->handleSelect = &handleSearchResultSelect;

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

static void freeSearchItems(struct Menu *self) {
	if(!self->items || !currentLazy->tracks)
		return;
	

}

static bool handleSearchResultSelect() {
	return false;
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
	//clearSearchDisplay(1);
	content->items = newLazy->tracks;
	currentLazy = newLazy;
	set_menu_items(content->menu, content->items);
	post_menu(content->menu);
	return true;
}

static void clearSearchDisplay(unsigned int startRow) {
	unsigned int line = startRow;
	int maxY;
	maxY = getmaxy(searchUi->window);
	while(line < maxY) {
		printf("AAAA\n");
		//wmove(searchUi->window, line, 0);  // Move to beginning of the line
       		//wclrtoeol(searchUi->window);  
		line++;
	}
}

