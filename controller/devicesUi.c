#include <ncurses.h>
#include <menu.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "spotifyCommands.h"
#include "devicesUi.h"
#include "ui.h"

static void closeDevicesWin();
static void freeDeviceArr(ITEM **devices);
static void handleKeypress(int key, char *sourceDir);
static bool display(char *sourceDir);
static void close();
static bool devicesSetItems(struct Menu *self, char *sourceDir);
static void devicesFreeItems(struct Menu *self);
static bool devicesHandleSelect(struct Menu *self, int key, char *sourceDir); 
static ITEM **makeDeviceArr(FILE *newLineList);

static struct Menu _devices = {
	.menu = NULL,
	.items = NULL, 
	.setItems = &devicesSetItems,
	.freeItems = &devicesFreeItems,
	.handleSelect = &devicesHandleSelect
};
static struct Menu *devices = &_devices;

void initializeDevicesWin() {
	devicesWin->display = &display;
	devicesWin->close = &close;
	devicesWin->handleKeypress = &handleKeypress;
}

static bool devicesSetItems(struct Menu *self, char *sourceDir) {
	FILE *devicesNewLineList = getDevicesNewLineList(sourceDir);
	if(devicesNewLineList == NULL)
		return false;

	self->items = makeDeviceArr(devicesNewLineList);
	fclose(devicesNewLineList);
	
	return self->items != NULL;
}

static void devicesFreeItems(struct Menu *self) {
	freeDeviceArr(self->items);
}

static bool devicesHandleSelect(struct Menu *self, int key, char *sourceDir) {
	ITEM *selection = current_item(self->menu);
	char *deviceId = item_userptr(selection);
        if(setActiveSpotifyDevice(deviceId))
		return true;

	return false;
}

static bool display(char *sourceDir) {
	if(!devices->setItems(devices, sourceDir))
		return false;

	devices->menu = assembleMenu(devices->items, devicesWin->window, 0, 0, "", false);
	if(devices->menu == NULL) {
		free_menu(devices->menu);
		freeDeviceArr(devices->items);
		return false;
	}

	return true;
}

static void close() {
	unpost_menu(devices->menu);
	free_menu(devices->menu);
	devices->freeItems(devices);
}

static void handleKeypress(int key, char *sourceDir) {
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
			devices->handleSelect(devices, key, sourceDir);
		break;
	}
}

static ITEM **makeDeviceArr(FILE *newLineList) {
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

static void freeDeviceArr(ITEM **devices) {
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

