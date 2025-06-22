#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <sys/wait.h>

int drawName(int row, int col);
int drawCurrentPlaying(int row, int col);
char *readAllFromFile(FILE *fp);

int main() {
	initscr();
	raw();
	drawName(0,0);
	drawCurrentPlaying(2,0);
	refresh();
	getch();
	endwin();
	return 0;
}

int drawName(int row, int col) {
	FILE *fp = popen("./scripts/getUserInfo", "r");
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

char *readAllFromFile(FILE *fp) {
	size_t bufferSize = 256;
	size_t position = 0;

	char *buffer = malloc(bufferSize);
	if (!buffer)
		return NULL;
	
	int thisChar = 0;
	while ((thisChar = fgetc(fp)) != EOF) {
		if (position+1 >= bufferSize) {
			bufferSize *= 2;
			char *newBuffer = realloc(buffer, bufferSize);
			if (!newBuffer) {
				free(buffer);
				return NULL;
			}
			buffer = newBuffer;
		}
		buffer[position++] = (char)thisChar;
	}
	buffer[position] = '\0';
	return buffer;
	
}

int drawCurrentPlaying(int row, int col) {
	FILE *fp = popen("./scripts/getCurrentPlaying", "r");

	if (fp) {
		char *newLineList = readAllFromFile(fp);
		int status = pclose(fp);
		if(newLineList == NULL || !WIFEXITED(status) || WEXITSTATUS(status)) {
			free(newLineList);	
			return 0;
		}
		move(row, col);
		printw("%s", newLineList);

		free(newLineList);
		return 1;
	}
	return 0;


}
