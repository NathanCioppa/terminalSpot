#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <sys/wait.h>
#include "config.h"
#include "utils.c"

int drawName(int row, int col);
char *readAllFromFile(FILE *fp);

int main() {
	initscr();
	raw();
	drawName(0,0);
	refresh();
	getch();
	endwin();
	return 0;
}

int drawName(int row, int col) {
	char *cmdArr[] = {"./scripts/getName", (char *)spotifyApiCmdDir, NULL};
	char *command = formatCommandArr(cmdArr);

	FILE *fp = popen(command, "r");
	char buffer[31]; // maximum allowed display name by spotify is 30 characters, 31 allows space for terminating char.

	if (fp) {
		char *result = fgets(buffer, sizeof(buffer), fp);
		int status = pclose(fp);
		// fgets fails, or the command exits abnormally, or exits with status 1; all indicate a failed execution.
		if(result == NULL || !WIFEXITED(status) || WEXITSTATUS(status))
			return 0;

		move(row,col);
		printw("%s", buffer);

		return 1;
	}
	return 0;
}
