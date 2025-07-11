#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <limits.h>
#include <ncurses.h>
#include "config.h"
#include "utils.h"
#include "ui.h"

int main() {
	char exePath[PATH_MAX];
	ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
	if(len == -1)
		return 1;
	exePath[len] = '\0';

	// Relevent files/commands are expected in certain places relevent to the location of the main executable.
	// Example: commands are expected in ./scripts/ where . is the location of the main executable. 
	// sourceDir is the absolute path to the directory containing the main executable.
	char *sourceDir = dirname(exePath);

	initscr();
	raw();
	noecho();
	refresh();
	initWindows();
	drawName(sourceDir);
	drawDevicesWin(sourceDir);
	endwin();

	char buf[PATH_MAX];

	getDirectSpotifyCmdPath(buf, sizeof(buf), "cmd");
	printf("%s", buf);

	return 0;
}
