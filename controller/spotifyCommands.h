#include <stdio.h>
#include <stdbool.h>

FILE *getDevicesNewLineList(char *sourceDir);
int setActiveSpotifyDevice(char *deviceId);
bool getName(char *nameBuffer, char *sourceDir);
FILE *getPlaylistsNewLineList(char *sourceDir, int limit, int offset);
int playContext(char *contextUri);
FILE *getAlbumsNewLineList(char *sourceDir, int limit, int offset); 
FILE *getShowsNewLineList(char *sourceDir, int limit, int offset); 
FILE *getEpisodesNewLineList(char *sourceDir, int limit, int offset); 
FILE *getArtistsNewLineList(char *sourceDir);
int playContextAt(char *contextUri, unsigned int startIdx);
int shuffleOff();
int shuffleOn();
FILE *directLoadPage(char *sourceDir, char *pageUrl); 
FILE *getLikedSongsNewLineList(char *sourceDir);
