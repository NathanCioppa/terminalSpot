#!/bin/bash

function newLineList() {
	[ "$#" -eq 0 ] && echo "Expected at least one argument" && exit 1
	list=""
	for arg in "$@"; do
		list+="${arg}"$'\n'
	done
	printf "%s" "${list}"
}
