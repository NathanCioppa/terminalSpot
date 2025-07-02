#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/authorization.sh"
source "$SCRIPT_DIR/handleRequestResponse.sh"

baseEndpoint="https://api.spotify.com/v1"

function requestPost() {
	endpoint="$1"
	
	response=$(curl -s -w "\n%{http_code}" --request POST \
		--url "$baseEndpoint$endpoint" \
		--header "Authorization: Bearer $accessToken"
	)

	checkResponse "$response"; exitStatus="$?"
	[[ "$exitStatus" -eq 64 ]] && refreshAccessToken && requestPost "$endpoint"
}
