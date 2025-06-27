#!/bin/bash

librespotEventsPipe="/tmp/librespotPipes/nowPlayingDisplay"
[ -d "$(dirname "$librespotEventsPipe")" ] || mkdir "$(dirname "$librespotEventsPipe")"
[ -p "$librespotEventsPipe" ] || mkfifo "$librespotEventsPipe"

function set_name() {
	name="$(grep '^NAME ' /tmp/librespotTrack | cut -d' ' -f2-)"
}
function set_artists() {
	artists="$(grep '^ARTIST ' /tmp/librespotTrack | cut -d' ' -f2-)"
}
function set_duration() {
	duration="$(grep '^DURATION ' /tmp/librespotTrack | cut -d' ' -f2-)"
}
function set_image() {
	imageUrl="$(grep '^IMAGE_URL ' /tmp/librespotTrack | cut -d' ' -f2-)"
	image="$(ascii-image-converter "$imageUrl" -C -c --color-bg -d 60,30)"
}
function set_position() {
	position="$(cat /tmp/librespotPosition)"
}
function set_pause_state() {
	pauseState="$(cat /tmp/librespotPauseState)"
}
function draw_display() {
	clear
	echo "$image"
	echo "$name"
	echo "$artists"

}
function initialize() {
	set_name
	set_artists
	set_duration
	set_image
	set_position
	set_pause_state

	draw_display
}

initialize

while true; do
	read event < "$librespotEventsPipe"
	case "$event" in
		"track_changed")
			set_name
			set_artists
			set_image
			set_duration ;;
		"playing" | "paused" | "seeked" | "position_corrected")
			set_position
			set_pause_state 
			set_name
			set_image
			set_artists;;
	esac
	draw_display
done
