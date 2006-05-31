#!/bin/sh
EXTRACT="`dirname $0`/extractmethodcalls.rb"
if test -n "$1" -a -f "$1"; then
	echo "preprocessing $1"
	cpp $1 2>/dev/null > tmp
	echo "extracting backend calls from $1"
	$EXTRACT tmp > tests/methods/$1
	rm tmp
else
	for i in *.cpp; do
		echo "preprocessing $i"
		cpp $i 2>/dev/null > tmp
		echo "extracting backend calls from $i"
		$EXTRACT tmp > tests/methods/$i
		rm tmp
	done
fi
