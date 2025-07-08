#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <menu.h>
#include "ui.h"
#include "utils.h"
#include "config.h"

static WINDOW *headerWin, *libraryWin, *searchWin, *devicesWin;
static int nameSize = 0;

ITEM **makeDeviceArr(FILE *newLineList);

void initWindows() {
	int headerHeight = 1;
	headerWin = newwin(headerHeight, COLS,0,0);
	devicesWin = newwin(LINES - headerHeight, COLS, headerHeight,0);
	keypad(devicesWin, TRUE);
}

int drawName() {
	char *cmdArr[] = {"./scripts/getName", (char *)spotifyApiCmdDir, NULL};
	char *command = formatCommandArr(cmdArr);

	FILE *fp = popen(command, "r");
	free(command);
	char buffer[31]; // maximum allowed display name by spotify is 30 characters, 31 allows space for terminating char.

	if (fp) {
		char *result = fgets(buffer, sizeof(buffer), fp);
		int status = pclose(fp);
		// fgets fails, or the command exits abnormally, or exits with status 1; all indicate a failed execution.
		if(result == NULL || !WIFEXITED(status) || WEXITSTATUS(status))
			return 0;

		nameSize = strlen(buffer);
		mvwprintw(headerWin,0,0,"%s",buffer);
		wrefresh(headerWin);

		return 1;
	}
	return 0;
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
				free(devices);
				return NULL;
			}
			devices = newDevices;
		}
		
		// Each device is 2 lines, first is name, second is id
		// id is what is needed from the selection, required to change spoify connect device
		if(nLine % 2 == 0) {
			devices[deviceIdx] = new_item(deviceName, "");

			char *deviceId = malloc(strlen(line)+1);
			strcpy(deviceId, line);
			set_item_userptr(devices[deviceIdx], deviceId);
			deviceIdx++;
		} 
		else {
			deviceName = strdup(line);
		}
		nLine++;
	}
	free(line);
	devices[deviceIdx] = NULL;
	return devices;
}

int drawDevicesWin() {
	char *cmdArr[] = {"./scripts/getDevices", (char *)spotifyApiCmdDir, NULL};
	char *command = formatCommandArr(cmdArr);

	FILE *fp = popen(command, "r");
	free(command);

	if(fp == NULL)
		return 0;
	
	ITEM **devices = makeDeviceArr(fp);
	if(devices == NULL) {
		fclose(fp);
		return 0;
	}
	fclose(fp);

	int height, width;
	getmaxyx(devicesWin, height, width);

	MENU *deviceMenu = new_menu(devices);
	set_menu_win(deviceMenu, devicesWin);
	set_menu_sub(deviceMenu, derwin(devicesWin, height, width, 0, 0));
	post_menu(deviceMenu);
	wrefresh(devicesWin);

	int ch;
	ITEM *selection;
	while ((ch = wgetch(devicesWin)) != 'q') {  // or some quit key
    		switch(ch) {
        		case KEY_DOWN:
            			menu_driver(deviceMenu, REQ_DOWN_ITEM);
            			break;
        		case KEY_UP:
            			menu_driver(deviceMenu, REQ_UP_ITEM);
            			break;
        		case 10: // Enter key
				selection = current_item(deviceMenu);
            			printf("%s", (char *)item_userptr(selection));
            		break;
    		}
    		wrefresh(devicesWin);
	}


	return 1;	
}
