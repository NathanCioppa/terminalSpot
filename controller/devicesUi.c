#include <ncurses.h>
#include <menu.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "spotifyCommands.h"
#include "ui.h"

ITEM **makeDeviceArr(FILE *newLineList);
void freeDeviceArr(ITEM **devices);
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
