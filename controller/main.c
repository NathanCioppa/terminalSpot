#include <ncurses.h>
#include "config.h"
#include "utils.h"
#include "ui.h"

int main() {
	initscr();
	raw();
	drawName();
	refresh();
	getch();
	endwin();
	return 0;
}
