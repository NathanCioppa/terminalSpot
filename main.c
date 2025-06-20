#include <ncurses.h>
#include <sys/wait.h>

int drawName(int x, int y);

int main() {
	initscr();
	raw();
	drawName(0,0);
	refresh();
	getch();
	endwin();
	return 0;
}

int drawName(int x, int y) {
	FILE *fp = popen("./scripts/getUserInfo", "r");
	char buffer[31]; // maximum allowed display name by spotify is 30 characters, 31 allows space for terminating char.

	if (fp) {
		char *result = fgets(buffer, sizeof(buffer), fp);
		int status = pclose(fp);
		// fgets fails, or the command exits abnormally, or exits with status 1; all indicate a failed execution.
		if(result == NULL || !WIFEXITED(status) || WEXITSTATUS(status))
			return 0;

		move(x,y);
		printw("%s", buffer);

		return 1;
	}
	return 0;
}
