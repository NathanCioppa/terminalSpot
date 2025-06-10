#!/bin/bash

configFile="$HOME/.config/terminalSpot/apiConfig.sh"
if [[ -f "$configFile" ]]; then
	source "$configFile" # grants access to $clientId $clientSecret $refreshToken if configured correctly.
else
	echo 0
	exit 1
fi

function refreshAccessToken() {
	response=$(curl -s -w "\n%{http_code}" -X POST https://accounts.spotify.com/api/token \
  		--user "$clientId:$clientSecret" \
		-d grant_type=refresh_token \
		-d refresh_token="$refreshToken"
	)
	httpCode=$(echo "$response" | tail -n1)
	body=$(echo "$response" | sed '$d')

	if [[ "$httpCode" -eq 200 ]] && jq -e . >/dev/null 2>&1 <<< "$body"; then
		jq -r ".access_token" <<< "$body"
	else
		echo 0
		exit 1
	fi
}
