#include <ncurses.h>
#include <menu.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "spotifyCommands.h"
#include "devicesUi.h"
#include "ui.h"

bool drawDevicesWin(char *sourceDir);
void closeDevicesWin();
void freeDeviceArr(ITEM **devices);
void handleKeypress(int key, char *sourceDir);
bool display(char *sourceDir);
void close();
bool devicesSetItems(struct Menu *self, char *sourceDir);
void devicesFreeItems(struct Menu *self);
ITEM **makeDeviceArr(FILE *newLineList);

struct Menu _devices = {
	.menu = NULL,
	.items = NULL, 
	.setItems = &devicesSetItems,
	.freeItems = &devicesFreeItems
};

struct Menu *devices = &_devices;

void initializeDevicesWin() {
	devicesWin->display = &display;
	devicesWin->close = &close;
	devicesWin->handleKeypress = &handleKeypress;
}

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

bool display(char *sourceDir) {
	FILE *devicesNewLineList = getDevicesNewLineList(sourceDir);
	if(devicesNewLineList == NULL)
		return false;

	ITEM **items = makeDeviceArr(devicesNewLineList);
	if(items == NULL)
		return false;

	devices->items = items;
	devices->menu = assembleMenu(items, devicesWin->window, 0, 0, "", false);
	if(devices->menu == NULL)
		return false;

	wrefresh(devicesWin->window);
	return true;
}

void close() {
	unpost_menu(devices->menu);
	free_menu(devices->menu);
	devices->freeItems(devices);
	wrefresh(devicesWin->window);
}

void handleKeypress(int key, char *sourceDir) {
	ITEM *selection;
	char *deviceId;
	switch(key) {
		case KEY_DOWN:
			menu_driver(devices->menu, REQ_DOWN_ITEM);
			wrefresh(devicesWin->window);
			break;
		case KEY_UP:
			menu_driver(devices->menu, REQ_UP_ITEM);
			wrefresh(devicesWin->window);
			break;
		case 10:
			selection = current_item(devices->menu);
			deviceId = item_userptr(selection);
            		setActiveSpotifyDevice(deviceId);
			wrefresh(devicesWin->window);
		break;
	}
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

/*
bool drawDevicesWin(char *sourceDir) {
	//FILE *devicesNewLineList = getDevicesNewLineList(sourceDir);
	//if(devicesNewLineList == NULL)
	//	return false;
	
	//ITEM **devices = makeDeviceArr(devicesNewLineList);
	//fclose(devicesNewLineList);
	//if(devices == NULL)
	//	return false;
	
	ITEM **deviceItems;
	devices->items = deviceItems;
	devices->setItems(devices, sourceDir);

	deviceMenu = assembleMenu(devices->items, devicesWin->window, 0, 0, "", false);
	wrefresh(devicesWin->window);

	int ch;
	ITEM *selection;
	char *selectedDeviceId;
	bool running = true;
	while (running && (ch = wgetch(devicesWin->window))) {
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
			default:
				running = universalControls(ch, devicesWin, sourceDir);
            		break;
    		}
    		wrefresh(devicesWin->window);
	}
	unpost_menu(deviceMenu);
	free_menu(deviceMenu);
	devices->freeItems(devices);

	return true;	
}

*/
