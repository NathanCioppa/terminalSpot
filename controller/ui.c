#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "ui.h"
#include "utils.h"
#include "config.h"

static WINDOW *headerWin, *libraryWin, *searchWin, *devicesWin;
static int nameSize = 0;

void initWindows() {
	int headerHeight = 1;
	headerWin = newwin(headerHeight, COLS,0,0);
	//devicesWin = newwin(LINES - headerHeight, COLS, headerHeight,0);
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
