#!/bin/bash

# expects some valid json object with an album array in it

function getAlbumImageUrl() {
	echo "$1" | jq -r '.album.images[0].url' || echo "default image url here"
}

function getFirstImageUrl() {
	echo "$1" | jq -r '.images[0].url' || echo "default image url here"

}
