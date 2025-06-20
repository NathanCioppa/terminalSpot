#!/bin/bash

# takes in an array of artist json data and echo's one line of all the artist's names, separated by ", "
function pullArtistNames() {
	echo "$1" | jq -r '.artists | map(.name) | join(", ")' || exit 1
}
