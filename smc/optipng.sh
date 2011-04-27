#!/bin/sh
# Runs optipng on all png files in the current directory

total_size() {
	find . -name \*.png -exec du -k {} \; | {
		while read kb foo ; do
			total=$((${total} + ${kb}))
		done
		echo ${total}
	}
}

optipng_r() {
	local opts="${*:--q -o4}" old_size=`total_size`

	find . -name \*.png | while read f ; do
		echo "optipng ${opts} ${f}"
		optipng ${opts} -- "${f}" || return 1
	done || return 1

	echo
	echo "Estimated reduction: $((${old_size} - `total_size`))K"
}

optipng_r "${*}"
