#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "utils.h"
#include "config.h"

#include "spotifyCommands.h"

// returns the availible devices as a new line seperated list, as formatted by ./scripts/getDevices.
// caller is responsable for closing the FILE.
FILE *getDevicesNewLineList(char *sourceDir) {
	char cmdPath[PATH_MAX];
	getScriptPath(cmdPath, sizeof(cmdPath), sourceDir, "getDevices");
	
	// format cmd to "/path/to/scripts/getDevices spotifyApiCmdDir" to run command.
	char cmd[strlen(cmdPath) + strlen(spotifyApiCmdDir) + 2];
	snprintf(cmd, sizeof(cmd), "%s %s", cmdPath, spotifyApiCmdDir);

	return popen(cmd, "r");	
}

// returns the exit status of the spotifyCommands/setDevice command (0 if success).
int setActiveSpotifyDevice(char *deviceId) {
	char cmdPath[PATH_MAX];
	getDirectSpotifyCmdPath(cmdPath, sizeof(cmdPath), "setDevice");

	// format cmd as "/path/to/spotifyCommands/setDevice deviceId" to run command.
	char cmd[strlen(cmdPath) + strlen(deviceId) + 2];
	snprintf(cmd, sizeof(cmd), "%s %s", cmdPath, deviceId);

	return system(cmd);
}
