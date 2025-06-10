#!/bin/bash

source ./authorization.sh
baseEndpoint="https://api.spotify.com/v1"

authBear="Authorization: Bearer"
accessToken=$(refreshAccessToken)

function getUserInfo() {
	response=$(curl -s -w "\n%{http_code}" -X GET \
  	--url "$baseEndpoint"/me \
  	--header "$authBear $accessToken"
	)

	echo "$response"
}

getUserInfo
