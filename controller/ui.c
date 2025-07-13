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

ITEM **makeDeviceArr(FILE *newLineList);
void freeDeviceArr(ITEM **devices);

void initWindows() {
	int headerHeight = 1;
	headerWin = newwin(headerHeight, COLS,0,0);
	devicesWin = newwin(LINES - headerHeight, COLS, headerHeight,0);
	keypad(devicesWin, TRUE);
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

int drawDevicesWin(char *sourceDir) {
	FILE *devicesNewLineList = getDevicesNewLineList(sourceDir);
	if(devicesNewLineList == NULL)
		return 0;
	
	ITEM **devices = makeDeviceArr(devicesNewLineList);
	fclose(devicesNewLineList);
	if(devices == NULL)
		return 0;

	int height, width;
	getmaxyx(devicesWin, height, width);

	MENU *deviceMenu = new_menu(devices);
	set_menu_win(deviceMenu, devicesWin);
	set_menu_sub(deviceMenu, derwin(devicesWin, height, width, 0, 0));
	post_menu(deviceMenu);
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
	freeDeviceArr(devices);

	return 1;	
}
