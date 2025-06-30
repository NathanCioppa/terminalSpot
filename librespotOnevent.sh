#!/bin/bash

case "$PLAYER_EVENT" in
	"track_changed")
		{
		 echo "NAME $NAME"
		 [ "$ITEM_TYPE" = "Track" ] && echo "ARTIST $ARTISTS"
		 [ "$ITEM_TYPE" = "Episode" ] && echo "ARTIST $SHOW_NAME"
		 echo "DURATION $DURATION_MS"
		 echo "IMAGE_URL ${COVERS%%$'\n'*}"
		 echo "SPOTIFY_URI $URI"
		 echo "$URI" >> /home/nathan/uris
		} > /tmp/librespotTrack ;;

		"seeked" | "position_correction")
			echo "$POSITION_MS" > /tmp/librespotPosition ;;

		"playing" | "paused")
			 echo "$POSITION_MS" > /tmp/librespotPosition
			 echo "$PLAYER_EVENT" > /tmp/librespotPauseState ;;
esac

for file in /tmp/librespotPipes/*; do
	[[ -p "$file" ]] && echo "$PLAYER_EVENT" > "$file"
done
