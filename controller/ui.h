#include <ncurses.h>
#include <menu.h>
#include <stdbool.h>

struct Menu {
	ITEM **items;
	bool (*setItems)(struct Menu*, char*); // may need char* to sourceDir if executing commands.
	void (*freeItems)(struct Menu*);
};

bool drawName(char *sourceDir);
MENU *assembleMenu(ITEM **items, WINDOW *window, unsigned int row, unsigned int column, char *menuMark, bool minimize);

extern WINDOW *currentWin, *headerWin, *libraryWin, *searchWin, *devicesWin;

