#!/bin/bash

source ./authorization.sh
baseEndpoint="https://api.spotify.com/v1"

authBear="Authorization: Bearer"
accessToken=$(refreshAccessToken)

function getUserInfo() {
	response=$(requestGet "/me")

	echo "$response"
}

function requestGet() {
	endpoint="$1"
	response=$(curl -s -w "\n%{http_code}" -X GET \
		--url "$baseEndpoint$endpoint" \
		--header "$authBear $accessToken"
	)
	httpCode=$(echo "$response" | tail -n1)
	body=$(echo "$response" | sed '$d')

	if [[ "$httpCode" -eq 200 ]]; then
		# echo body only if it is valid JSON
		if jq -e . >/dev/null 2>&1 <<< "$body"; then
			echo "$body"
			exit 0
		else
			exit 1
		fi
	else # the request was unsuccessful
		exit 1
	fi
}

getUserInfo
