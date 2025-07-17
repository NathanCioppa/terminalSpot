#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <limits.h>
#include <ncurses.h>

#include "ui.h"
#include "libraryUi.h"
#include "devicesUi.h"

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

	initializeUi(sourceDir);
	startDefaultWindow(sourceDir);
	runUiLooper(sourceDir);

	endwin();
	return 0;
}

