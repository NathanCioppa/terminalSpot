#include <stdio.h>
#include <menu.h>

void getScriptPath(char *buffer, size_t bufferSize, char *sourceDir, char *scriptName);
void getDirectSpotifyCmdPath(char *buffer, size_t bufferSize, char *commandName);
char *readAllFromFile(FILE *fp);
char *formatCommandArr(char **command);

#ifndef TRACKER_H
#define TRACKER_H
struct LazyTracker {
	ITEM **tracks;
	char *nextPage;
	int limitPerRequest;
	bool (*expand)(struct LazyTracker *self, char *sourceDir);
	void (*clean)(struct LazyTracker *self);
};
#endif

bool lazyLoadTracks(struct LazyTracker *self, char *sourceDir);
struct LazyTracker *initLazyTracker(FILE *newLineList, int limitPerRequest, bool (*expand)(struct LazyTracker *self, char *sourceDir), void (*clean)(struct LazyTracker *self)); 
void cleanLazyLoadedTracks(struct LazyTracker *self);
void uriToId(char *idDest, char *uri); 

