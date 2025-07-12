#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <libgen.h>
#include <unistd.h>
#include <string.h>
#include "config.h"
#include "utils.h"

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
