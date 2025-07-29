#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <libgen.h>
#include <unistd.h>
#include <string.h>
#include "config.h"
#include "utils.h"
#include "spotifyCommands.h"

// Relevent files/commands are expected in certain places relevent to the location of the main executable.
// Example: commands are expected in ./scripts/ where . is the location of the main executable. 

// sourceDir is the absolute path to the directory containing the main executable.
// scriptName is the name of a script located at sourceDir/scripts/scriptName.
void getScriptPath(char *buffer, size_t bufferSize, char *sourceDir, char* scriptName) {
	snprintf(buffer, bufferSize, "%s/scripts/%s", sourceDir, scriptName);
}

// Commands that do not return anything just be run directly from spotifyApiCmdDir.
// Example: setting active spotify device (spotifyCommands/setDevice).
void getDirectSpotifyCmdPath(char *buffer, size_t bufferSize, char *commandName) {
	snprintf(buffer, bufferSize, "%s/%s", spotifyApiCmdDir, commandName);
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

// takes a null terminated array of strings; the command and its arguments in order.
char *formatCommandArr(char **command) {
	size_t position = 0; 
	size_t cmdStrSize = 128;
	char *cmdStr = malloc(cmdStrSize);
	if (!cmdStr)
		return NULL;

	int i = 0;
	while (command[i] != NULL) {
		char *arg = command[i];
		int j = 0;
		while (arg[j] != '\0') {
			int thisChar = arg[j];
			// ensure room for next char and a space char in case at end of arg 
			if(position+2 >= cmdStrSize) {
				cmdStrSize *= 2;
				char *newCmdStr = realloc(cmdStr, cmdStrSize);
				if(!newCmdStr) {
					free(cmdStr);
					return NULL;
				}
				cmdStr = newCmdStr;
			}
			cmdStr[position++] = (char)thisChar;
			j++;
		}
		cmdStr[position++] = ' '; // each arg separated by space, will result in trailing space char
		i++;
	}
	if (position > 0) {
		cmdStr[position-1] = '\0'; // replace the trailing space char with null terminator
		return cmdStr;
	} else {
		free(cmdStr);
		return NULL;
	}
}

struct LazyTracker *initLazyTracker(FILE *newLineList, int limitPerRequest, bool (*expand)(struct LazyTracker *self, char *sourceDir), void (*clean)(struct LazyTracker *self)) { 
	char *line = NULL;
	size_t len = 0;

	if(getline(&line, &len, newLineList) == -1) {
		fclose(newLineList);
		return NULL;
	}
	line[strcspn(line, "\n")] = '\0';
	char *nextPage = strdup(line);

	ITEM **tracks = calloc((limitPerRequest + 2), sizeof(ITEM *));
	if(!tracks) {
		fclose(newLineList);
		return NULL;
	}

	size_t trackIdx = 0;
	unsigned int trackLine = 1;
	char *trackName;
	char *trackDesc;
	char *trackUserPtr;

	while(getline(&line, &len, newLineList) != -1) {
		line[strcspn(line, "\n")] = '\0';
		for (char *p = line; *p; ++p) {
    			if ((unsigned char)*p >= 128) *p = '?';
		}
	
		if(trackLine == 1) {
			trackName = strdup(line);	
		}
		else if (trackLine == 2){
			trackDesc = strdup(line);
		}
		else {
			tracks[trackIdx] = new_item(trackName, trackDesc);
			set_item_userptr(tracks[trackIdx], strdup(line));
			trackIdx++;
		}
		
		trackLine = (trackLine + 1) % 3;
	}
	free(line);

	if(trackIdx >= limitPerRequest - 2) {
		tracks[trackIdx] = new_item("Load More Items ->", ".");
		tracks[trackIdx+1] = NULL;
	}
	else {
		tracks[trackIdx] = NULL;
	}

	struct LazyTracker *tracker = malloc(sizeof(struct LazyTracker));
	tracker->tracks = tracks;
	tracker->limitPerRequest = limitPerRequest;
	tracker->nextPage = nextPage;
	tracker->expand = expand;
	tracker->clean = clean;

	return tracker;
}

bool lazyLoadTracks(struct LazyTracker *self, char *sourceDir) {
	FILE *tracksNewLineList = directLoadPage(self->nextPage, sourceDir);

	if(!tracksNewLineList)
		return NULL;

	char *line = NULL;
	size_t len = 0;

	if(getline(&line, &len, tracksNewLineList) == -1) {
		fclose(tracksNewLineList);
		return false;
	}
	line[strcspn(line, "\n")] = '\0';
	
	bool isFinalPage = false;
	if(strcmp(line, "null") == 0) {
		self->nextPage = NULL;
		isFinalPage = true;
	}
	else
		self->nextPage = strdup(line);

	line[strcspn(line, "\n")] = '\0';
	if(strcmp(line, "null") == 0) {
		self->nextPage = NULL;
		isFinalPage = true;
	}
	else
		self->nextPage = strdup(line);

	ITEM **thisPage = malloc((self->limitPerRequest + 2) * sizeof(ITEM *));

	unsigned int trackLine = 1;
	char *trackName = NULL;
	char *trackDesc = NULL;
	char *trackUserPtr = NULL;
	size_t trackIdx = 0;
	while(getline(&line, &len, tracksNewLineList) != -1) {
		line[strcspn(line, "\n")] = '\0';
		for (char *p = line; *p; ++p) {
    			if ((unsigned char)*p >= 128) *p = '?';
		}
	
		if(trackLine == 1) {
			trackName = strdup(line);
		}
		else if (trackLine == 2){
			trackDesc = strdup(line);
		}
		else {
			ITEM *track = new_item(trackName, trackDesc);
			if(!track) {
				trackLine = (trackLine + 1) % 3;
				continue;
			} 
			set_item_userptr(track, strdup(line));
			thisPage[trackIdx] = track;
			trackIdx++;
		}
		trackLine = (trackLine + 1) % 3;
	}
	free(line);
	if(isFinalPage)
		thisPage[trackIdx] = NULL;
	else {
		thisPage[trackIdx] = new_item("Load More Items ->", ".");
		thisPage[trackIdx+1] = NULL;
	}

	size_t i = 0;
	while(self->tracks[i] && (strcmp(item_description(self->tracks[i]), ".") != 0) )
		i++;
	size_t priorPageSize = i;

	i = 0;
	while(thisPage[i])
		i++;
	size_t thisPageSize = i;

	ITEM **expandedLazyItems = malloc((priorPageSize + thisPageSize + 1) * sizeof(ITEM *));

	for(i=0; i<priorPageSize; i++)
		expandedLazyItems[i] = self->tracks[i];
	for(i=priorPageSize; i<priorPageSize+thisPageSize; i++)
		expandedLazyItems[i] = thisPage[i - priorPageSize];
	expandedLazyItems[i] = NULL;

	free(thisPage);
	free(self->tracks);
	self->tracks = expandedLazyItems;

	return true;
}

void cleanLazyLoadedTracks(struct LazyTracker *self) {
	ITEM *item;
	for(size_t i = 0; (item = self->tracks[i]); i++) {
		if(strcmp(item_description(item), ".") == 0) {
			free_item(item);
			continue;
		}

		free((char *)item_name(item));
		free((char *)item_description(item));
		free(item_userptr(item));
		free_item(item);
	}
	free(self->tracks);
}

void uriToId(char *idDest, char *uri) {
	int colonCount = 0;
	int idIdx = 0;
	for(int i=0; uri[i]; i++) {
		if(colonCount == 2) {
			idDest[idIdx] = uri[i];
			idIdx++;
			continue;
		}
		if(uri[i] == ':'){
			colonCount++;
			continue;
		}
	}
	idDest[idIdx] = '\0';
}
