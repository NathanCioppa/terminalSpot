#!/bin/bash

function checkResponse() {
	response="$1"

	httpCode=$(echo "$response" | tail -n1)
	body=$(echo "$response" | sed '$d')

	case "$httpCode" in
		"200") # echo body only if it is valid JSON
			if jq -e . >/dev/null 2>&1 <<< "$body"; then
				echo "$body"
				exit 0
			else
				exit 1
			fi ;;
		"204") # put/post request sent, not output to write
			exit 0 ;;
		"401") # access token was invalid, but this can be handled, so dont exit
			return 64 ;;
	esac
	
	# should exit (or return) in one of the cases above, assume failiure if not
	exit 1
}
