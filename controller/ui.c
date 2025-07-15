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

static WINDOW *headerWin, *libraryWin, *searchWin, *devicesWin;
static int nameSize = 0;
static unsigned int libraryFilterMenuWidth = 0;


struct Menu {
	ITEM **items;
	bool (*setItems)(struct Menu*, char*); // may need char* to sourceDir if executing commands.
	void (*freeItems)(struct Menu*);
};



ITEM **makeDeviceArr(FILE *newLineList);
void freeDeviceArr(ITEM **devices);
size_t getMinMenuWidth(ITEM **items, size_t menuMarkLen);
void freeLibraryFilterItems(ITEM **filters);
MENU *assembleMenu(ITEM **items, WINDOW *window, unsigned int row, unsigned int column, char *menuMark, bool minimize);
bool devicesSetItems(struct Menu *self, char *sourceDir);
void devicesFreeItems(struct Menu *self);


bool devicesSetItems(struct Menu *self, char *sourceDir) {
	FILE *devicesNewLineList = getDevicesNewLineList(sourceDir);
	if(devicesNewLineList == NULL)
		return false;

	self->items = makeDeviceArr(devicesNewLineList);
	fclose(devicesNewLineList);
	
	return self->items != NULL;
}

void devicesFreeItems(struct Menu *self) {
	freeDeviceArr(self->items);
}



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

int drawDeviceName() {
	move(0,nameSize);
	clrtoeol();
	move(0,nameSize);

	return 1;
}

ITEM **makeDeviceArr(FILE *newLineList) {
	size_t nItems = 5;
	ITEM **devices = calloc(nItems, sizeof(ITEM *));
	if (devices == NULL)
		return NULL;
	
	unsigned int nLine = 1;
	unsigned int deviceIdx = 0;
	char *deviceName;
	char *line = NULL;
	size_t len = 0;

	int i = 0;
	while(getline(&line, &len, newLineList) != -1) {
		// remove trailing new line char
		line[strcspn(line, "\n")] = '\0';

		// make sure there is space for another item & NULL
		if (deviceIdx+1 >= nItems) {
			nItems *= 2;
			ITEM **newDevices = realloc(devices, nItems * sizeof(ITEM *));
			if (newDevices == NULL) {
				devices[deviceIdx] = NULL;
				freeDeviceArr(devices);
				free(line);
				return NULL;
			}
			devices = newDevices;
		}
		
		// Each device is 2 lines, first is name, second is id
		// id is what is needed from the selection, required to change spoify connect device
		if(nLine % 2 == 0) {
			devices[deviceIdx] = new_item(deviceName, "desc");

			char *deviceId = malloc(strlen(line)+1);
			if (deviceId == NULL) {
				devices[deviceIdx + 1] = NULL;
				freeDeviceArr(devices);
				free(deviceName); // would have been set in prior loop, but never assigned to a device
				free(line);
				return NULL;
			}
			strcpy(deviceId, line);
			set_item_userptr(devices[deviceIdx], deviceId);
			deviceIdx++;
		} 
		else {
			deviceName = strdup(line);
			if(deviceName == NULL) {
				devices[deviceIdx + 1] = NULL;
				freeDeviceArr(devices);
				free(line);
				return NULL;
			}
		}
		nLine++;
	}
	free(line);
	devices[deviceIdx] = NULL;
	return devices;
}

void freeDeviceArr(ITEM **devices) {
	unsigned int i = 0;
	ITEM *device;
	while((device = devices[i]) != NULL) {
		free(item_userptr(device));
		free((char *)item_name(device)); // name is originally passed by strdup(), nesecary to free
		free_item(device);
		i++;
	}
	free(devices);
}

bool drawDevicesWin(char *sourceDir) {
	//FILE *devicesNewLineList = getDevicesNewLineList(sourceDir);
	//if(devicesNewLineList == NULL)
	//	return false;
	
	//ITEM **devices = makeDeviceArr(devicesNewLineList);
	//fclose(devicesNewLineList);
	//if(devices == NULL)
	//	return false;
	
	ITEM **deviceItems;
	struct Menu devices = {deviceItems, &devicesSetItems, &devicesFreeItems};
	devices.setItems(&devices, sourceDir);

	MENU *deviceMenu = assembleMenu(devices.items, devicesWin, 0, 0, "", false);
	wrefresh(devicesWin);

	int ch;
	ITEM *selection;
	char *selectedDeviceId;
	bool running = true;
	while (running && (ch = wgetch(devicesWin))) {
    		switch(ch) {
        		case KEY_DOWN:
            			menu_driver(deviceMenu, REQ_DOWN_ITEM);
            			break;
        		case KEY_UP:
            			menu_driver(deviceMenu, REQ_UP_ITEM);
            			break;
        		case 10: // Enter key
				selection = current_item(deviceMenu);
				selectedDeviceId = item_userptr(selection);
            			setActiveSpotifyDevice(selectedDeviceId);

				break;
			case 'q': // quit application
				running = false;
            		break;
    		}
    		wrefresh(devicesWin);
	}
	unpost_menu(deviceMenu);
	free_menu(deviceMenu);
	devices.freeItems(&devices);

	return true;	
}

// Handles constructing a posting a menu of given items to a given window. Will not refresh window after posting. 
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

bool drawLibraryWin(char *sourceDir) {
	unsigned int filterSize = 6;
	ITEM *filters[filterSize + 1];
	filters[albumsIdx] = new_item("Albums","");
	filters[artistsIdx] = new_item("Artists", "");
	filters[playlistsIdx] = new_item("Playlists","");
	filters[showsIdx] = new_item("Shows", "");
	filters[episodesIdx] = new_item("Episodes", "");
	filters[likedSongsIdx] = new_item("Liked Songs", "");
	filters[filterSize] = NULL;


	MENU *filterMenu = assembleMenu(filters, libraryWin, 0, 0, "", true);
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
	freeLibraryFilterItems(filters);

	return true;
}

/*
void handleFilterSelection(const unsigned int selectedIdx) {
	if (selectedIdx == albumsIdx)
		showAlbums();
	else if (selectedIdx == artistsIdx)
		showArtists();
	else if (selectedIdx == playlistsIdx)
		showPlaylists();
	else if (selectedIdx == showsIdx)
		showShows();
	else if (selectedIdx == episodesIdx)
		showEpisodes();
	else if (selectedxIdx == likedSongsIdx)
		showLikedSongs();
}
*/

void freeLibraryFilterItems(ITEM **filters) {
	unsigned int i = 0;
	ITEM *thisFilter;
	while((thisFilter = filters[i]) != NULL) {
		free(thisFilter);
		i++;
	}
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
