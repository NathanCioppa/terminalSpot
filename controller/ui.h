#include <ncurses.h>
#include <menu.h>
#include <stdbool.h>
#include "utils.h"

#ifndef MENU_H
#define MENU_H
struct Menu {
	MENU *menu;
	ITEM **items;
	bool (*setItems)(struct Menu *self, char *sourceDir); // may need char* to sourceDir if executing commands.
	void (*freeItems)(struct Menu *self);
	bool (*handleSelect)(struct Menu *self, int key, char *sourceDir);
};
#endif

#ifndef WINDOW_H
#define WINDOW_H
struct Window {
	WINDOW *window;
	bool (*display)(char *sourceDir);
	void (*close)();
	void (*handleKeypress)(int key, char *sourceDir);
};
#endif

extern struct Window *currentWin, *libraryWin, *searchWin, *devicesWin;

MENU *assembleMenu(ITEM **items, WINDOW *window, unsigned int row, unsigned int column, char *menuMark, bool minimize);
bool initializeUi(char *sourceDir);
bool startDefaultWindow(char *sourceDir);
void runUiLooper(char *sourceDir);

bool setCurrentLazy(FILE *lazyInitNewLineList, struct LazyTracker *(*initFunction)(FILE *newLineList, int limitPerRequest, bool (*expand)(struct LazyTracker *self, char *sourceDir), void (*clean)(struct LazyTracker *self)), bool (*expand)(struct LazyTracker *self, char *sourceDir), void (*clean)(struct LazyTracker *self));
void closeCurrentLazy();

extern struct LazyTracker *currentLazyTracker;
extern char *lazyContext;
extern char *backLazyContext;
extern struct Menu *backLazy;
extern struct LazyTracker *backLazyTracker;
