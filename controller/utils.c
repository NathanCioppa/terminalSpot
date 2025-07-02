#include <stdio.h>
#include <stdlib.h>

char *readAllFromFile(FILE *fp);
char *runCmd(char **command);

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

// takes a null terminated array of strings, the command and its arguments in order.
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
