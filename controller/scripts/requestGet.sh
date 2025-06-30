#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/authorization.sh"
source "$SCRIPT_DIR/handleRequestResponse.sh"

baseEndpoint="https://api.spotify.com/v1"

function requestGet() {
	endpoint="$1"

	response=$(curl -s -w "\n%{http_code}" -X GET \
		--url "$baseEndpoint$endpoint" \
		--header "Authorization: Bearer $accessToken"
	)

	body=$(checkResponse "$response") # echo's the body if successful, error code 64 means the access token is expired
	case "$?" in
		"0") echo "$body" ;;
		"64") 
		     refreshAccessToken
		     requestGet "$endpoint" ;;
		*) exit 1 ;;
	esac
}
