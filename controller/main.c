#include <ncurses.h>
#include "config.h"
#include "utils.h"
#include "ui.h"

int main() {
	initscr();
	raw();
	refresh();
	initWindows();
	drawName();
	getch();
	endwin();
	return 0;
}
