#include <stdio.h>
#include <menu.h>

void getScriptPath(char *buffer, size_t bufferSize, char *sourceDir, char *scriptName);
void getDirectSpotifyCmdPath(char *buffer, size_t bufferSize, char *commandName);
char *readAllFromFile(FILE *fp);
char *formatCommandArr(char **command);

#ifndef TRACKER_H
#define TRACKER_H
struct TrackTracker {
	ITEM **tracks;
	char *nextPage;
	int limitPerRequest;
};
#endif

bool lazyLoadTracks(struct TrackTracker *self, char *sourceDir);
struct TrackTracker *initLazyTracks(FILE *newLineList, int limitPerRequest, char *sourceDir); 

