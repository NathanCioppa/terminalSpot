#include <stdio.h>
#include <stdbool.h>

FILE *getDevicesNewLineList(char *sourceDir);
int setActiveSpotifyDevice(char *deviceId);
bool getName(char *nameBuffer, char *sourceDir);
FILE *getPlaylistsNewLineList(char *sourceDir, int limit, int offset);
int playContext(char *contextUri);
FILE *getAlbumsNewLineList(char *sourceDir, int limit, int offset); 


