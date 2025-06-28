#!/bin/bash

librespotEventsPipe="/tmp/librespotPipes/nowPlayingDisplay"
[ -d "$(dirname "$librespotEventsPipe")" ] || mkdir "$(dirname "$librespotEventsPipe")"
[ -p "$librespotEventsPipe" ] || mkfifo "$librespotEventsPipe"

function set_name() {
	name="$(grep '^NAME ' /tmp/librespotTrack | cut -d' ' -f2-)"
	draw_name
}
function draw_name() {
	nameLength="${#name}"
	# dont let string wrap to the next line
	[[ "$nameLengh" -gt "$cols" ]] && name="${name:0:cols}" && nameLength="$cols"
	# position so string is centered
	tput cup "$firstInfoRow" "$(( $((cols / 2)) - $((nameLength / 2)) ))"
	tput el; tput el1 # clear entire line
	tput bold
	echo "$name"
	tput sgr0
}

function set_artists() {
	artists="$(grep '^ARTIST ' /tmp/librespotTrack | cut -d' ' -f2-)"
	draw_artists
}
function draw_artists() {
	artistsLength="${#artists}"
	[[ "$artistsLength" -gt "$cols" ]] && artists="${artists:0:cols}" && artistsLength="$cols" 
	tput cup "$((firstInfoRow + 1))" "$(( $((cols / 2)) - $((artistsLength / 2)) ))"
	tput el; tput el1
	echo "$artists"
}

function set_image() {
	imageUrl="$(grep '^IMAGE_URL ' /tmp/librespotTrack | cut -d' ' -f2-)"
	# full contents have some junk in the first line, must be discarded else padding is messed up.
	image="$(ascii-image-converter "$imageUrl" -C -c --color-bg -d "$((imageSize * 2)),$imageSize" | tail -n +2)"
	# image will not print correctly by changing column with tput, so pad columns manually
	paddedImage="$(printf '%s\n' "$image" | sed "s/^/$(printf '%*s' "$imageCol")/")"
	draw_image
}
function draw_image() {
	tput cup "$imageRow" 0 
	printf "%b\n" "$paddedImage"
	tput cup "$((rows - minInfoRows))" 0
}

function set_duration() {
	durationMs="$(grep '^DURATION ' /tmp/librespotTrack | cut -d' ' -f2-)"
	durationFullSecs="$((durationMs / 1000))"
	durationOverflowSecs="$((durationFullSecs % 60))"
	[[ "$durationOverflowSecs" -lt 10 ]] && durationOverflowSecs="0$durationOverflowSecs"
	durationDisplay="$((durationFullSecs / 60)):$durationOverflowSecs"
	draw_duration
}
function draw_duration() {
	durationDisplaySize=${#durationDisplay}
	durationCol="$((cols - durationDisplaySize))"
	tput cup "$((firstInfoRow + 2 ))" "$durationCol"
	echo "$durationDisplay"
}
function set_position() {
	positionMs="$(cat /tmp/librespotPosition)"
	draw_position
}
function draw_position() {
	positionFullSecs="$((positionMs / 1000))"
	positionOverflowSecs="$((positionFullSecs % 60))"
	[[ "$positionOverflowSecs" -lt 10 ]] && positionOverflowSecs="0$positionOverflowSecs"
	positionDisplay="$((positionFullSecs / 60)):$positionOverflowSecs"
	positionDisplaySize=${#positionDisplay}
	tput cup "$((firstInfoRow + 2))" 0
	echo "$positionDisplay"
}

function set_pause_state() {
	pauseState="$(cat /tmp/librespotPauseState)"
}
# Clear and redraw all elements. Usually done to adjust after a resize to clear prevent junk.
function reset_display() {
	tput clear
	draw_name
	draw_artists
	set_duration
	draw_position
	set_image # set_image handles sizing the image, so just calling draw_image is not enough.
}

function set_size_vars() {
	minInfoRows=4 # number of lines below album art that should be free for other info.
	rows="$(tput lines)"
	cols="$(tput cols)"
	firstInfoRow="$((rows - minInfoRows))"

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
	set_image
	set_position
#	set_pause_state

	reset_display
}

initialize

while true; do
	read event < "$librespotEventsPipe"
	if [ "$(tput lines)" != "$rows" ] || [ "$(tput cols)" != "$cols" ]; then
		set_size_vars
		reset_display
	fi
	case "$event" in
		# track_changed and playing usually get sent at about the same time, with playing always being called last.
		"track_changed")
			set_name
			set_artists
			set_duration
			set_image & ;; # takes a while, do it in background
		"playing" | "paused")
			set_position ;;

		"seeked" | "position_correction")
			set_position ;;
	esac
done
