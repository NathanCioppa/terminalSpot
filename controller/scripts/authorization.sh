#!/bin/bash

# Whenever this file is sourced, it grants access to the user's access token 
# by trying to set the accessToken from a temp file.
# 
# If that file does not exist, refreshToken is called to get a new token
# and write it to the temp file.
#
# Any file that requires the accessToken should source this file.

function refreshAccessToken() {
	response=$(curl -s -w "\n%{http_code}" -X POST https://accounts.spotify.com/api/token \
  		--user "$clientId:$clientSecret" \
		-d grant_type=refresh_token \
		-d refresh_token="$refreshToken"
	)
	httpCode=$(echo "$response" | tail -n1)
	body=$(echo "$response" | sed '$d')

	if [[ "$httpCode" -eq 200 ]] && jq -e . >/dev/null 2>&1 <<< "$body"; then
		jq -r ".access_token" <<< "$body" > "$tmpAccessTokenFile" 
		accessToken="$(cat "$tmpAccessTokenFile")" 
	else
		exit 1
	fi
}

configFile="$HOME/.config/terminalSpot/apiConfig.sh"
if [[ -f "$configFile" ]]; then
	source "$configFile" # grants access to $clientId $clientSecret $refreshToken if configured correctly.
else
	exit 1
fi

tmpAccessTokenFile="/tmp/terminalSpotAccessToken"

if [ -e "$tmpAccessTokenFile" ]; then 
	accessToken=$(cat "$tmpAccessTokenFile")
else 
	refreshAccessToken
fi
