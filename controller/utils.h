#include <stdio.h>

void getScriptPath(char *buffer, size_t bufferSize, char *sourceDir, char *scriptName);
void getDirectSpotifyCmdPath(char *buffer, size_t bufferSize, char *commandName);
char *readAllFromFile(FILE *fp);
char *formatCommandArr(char **command);

