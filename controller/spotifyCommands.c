#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include "utils.h"
#include "config.h"

#include "spotifyCommands.h"

// Returns the availible devices as a new line seperated list, as formatted by ./scripts/getDevices.
// Caller is responsable for closing the FILE.
FILE *getDevicesNewLineList(char *sourceDir) {
	char cmdPath[PATH_MAX];
	getScriptPath(cmdPath, sizeof(cmdPath), sourceDir, "getDevices");
	
	// format cmd to "/path/to/scripts/getDevices spotifyApiCmdDir" to run command.
	char cmd[strlen(cmdPath) + strlen(spotifyApiCmdDir) + 2];
	snprintf(cmd, sizeof(cmd), "%s %s", cmdPath, spotifyApiCmdDir);

	return popen(cmd, "r");	
}

// Returns the exit status of the spotifyCommands/setDevice command (0 if success).
int setActiveSpotifyDevice(char *deviceId) {
	char cmdPath[PATH_MAX];
	getDirectSpotifyCmdPath(cmdPath, sizeof(cmdPath), "setDevice");

	// format cmd as "/path/to/spotifyCommands/setDevice deviceId" to run command.
	char cmd[strlen(cmdPath) + strlen(deviceId) + 2];
	snprintf(cmd, sizeof(cmd), "%s %s", cmdPath, deviceId);

	return system(cmd);
}

// true returned if *nameBuffer is set to the result of scripts/getName.
bool getName(char *nameBuffer, char *sourceDir) {
	char cmdPath[PATH_MAX];
	getScriptPath(cmdPath, sizeof(cmdPath), sourceDir, "getName");
	
	// format cmd as "/path/to/scripts/getName spotifyApiCmdDir" to run command.
	char cmd[strlen(cmdPath) + strlen(spotifyApiCmdDir) + 2];
	snprintf(cmd, sizeof(cmd), "%s %s", cmdPath, spotifyApiCmdDir);

	FILE *fp = popen(cmd, "r");
	if (fp == NULL)
		return false;

	char *result = fgets(nameBuffer, sizeof(nameBuffer), fp);
	int status = fclose(fp);

	// fgets fails, or the command exits abnormally, or exits with status 1; all indicate a failed execution.
	if(result == NULL || !WIFEXITED(status) || WEXITSTATUS(status))
		return false;

	return true;
}
