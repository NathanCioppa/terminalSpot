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
function draw_image() {
	imageUrl="$(grep '^IMAGE_URL ' /tmp/librespotTrack | cut -d' ' -f2-)"
	# full contents have some junk in the first line, must be discarded else padding is messed up.
	image="$(ascii-image-converter "$imageUrl" -C -c --color-bg -d "$((imageSize * 2)),$imageSize" | tail -n +2)"
	# image will not print correctly by changing column with tput, so pad columns manually
	paddedImage="$(printf '%s\n' "$image" | sed "s/^/$(printf '%*s' "$imageCol")/")"
	
	tput cup "$imageRow" 0 
	printf "%b\n" "$paddedImage"
	tput cup "$((rows - minInfoRows))" 0

}
function set_position() {
	position="$(cat /tmp/librespotPosition)"
}
function set_pause_state() {
	pauseState="$(cat /tmp/librespotPauseState)"
}
function draw_display() {
	clear
	draw_image
	echo "$name"
	echo "$artists"

}


function set_size_vars() {
	minInfoRows=3 # number of lines below album art that should be free for other info.
	rows="$(tput lines)"
	cols="$(tput cols)"

	# image will be a square, imageSize is length of each side in row heights,
	# (assuming column width is half of a row's height)
	if (( "$((rows - minInfoRows))" < "$((cols / 2))" )); then
		imageSize="$((rows - minInfoRows))" 
		imageRow=0
		imageCol="$(( $((cols / 2)) - imageSize ))"
	else 
		imageSize="$((cols / 2))"
		imageRow="$(( $(( $(( rows - minInfoRows )) / 2 )) - $(( imageSize / 2 )) ))"
		imageCol=0
	fi
}

function initialize() {
	set_size_vars
	set_name
	set_artists
	set_duration
	draw_image
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
	set_size_vars
	draw_display
done
