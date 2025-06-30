#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/authorization.sh"
source "$SCRIPT_DIR/handleRequestResponse.sh"

baseEndpoint="https://api.spotify.com/v1"

function requestPut() {
	endpoint="$1"
	data="$2" # should be json object

	if [ -z "$data" ]; then
		response=$(curl -s -w "\n%{http_code}" --request PUT \
			--url "$baseEndpoint$endpoint" \
			--header "Authorization: Bearer $accessToken"
		)
	else
		response=$(curl -s -w "\n%{http_code}" --request PUT \
			--url "$baseEndpoint$endpoint" \
			--header "Authorization: Bearer $accessToken" \
			--header 'Content-Type: application/json' \
			--data "$data" 
		)
	fi

	checkResponse "$response"; exitStatus="$?"
	[[ "$exitStatus" -eq 64 ]] && refreshAccessToken && requestPut "$endpoint" "$data"
}
