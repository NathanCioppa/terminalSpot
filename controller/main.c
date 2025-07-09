#include <ncurses.h>
#include "config.h"
#include "utils.h"
#include "ui.h"

int main() {
	initscr();
	raw();
	noecho();
	refresh();
	initWindows();
	drawName();
	drawDevicesWin();
	endwin();
	return 0;
}
