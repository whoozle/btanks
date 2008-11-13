#!/bin/sh

test() {
	prefix=$1
	tiles="$prefix/tiles";
	echo $tiles;
	for f in `find "$tiles" -iname '*.png' | xargs`; do 
		if [ ! -f "$f.map" ]; then echo $f; fi
	done

	return 0;
}

test "data";

