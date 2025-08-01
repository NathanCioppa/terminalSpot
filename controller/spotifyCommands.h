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
FILE *getLikedSongsNewLineList(char *sourceDir, unsigned int limit);
int playTrack(char *trackUri); 
FILE *getArtistAlbumsNewLineList(char *artistId, unsigned int limit, char *sourceDir);
FILE *getAlbumTracksNewLineList(char *albumId, char *sourceDir); 
FILE *getPlaylistTracksNewLineList(char *playlistId, unsigned int limit, char *sourceDir); 
FILE *getShowEpisodesNewLineList(char *showId, unsigned int limit, char *sourceDir); 
