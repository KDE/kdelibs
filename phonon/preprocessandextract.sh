#!/bin/sh
EXTRACT="`dirname $0`/extractmethodcalls.rb"
IGNORE="^\(audiodevice.cpp\|audiodeviceenumerator.cpp\|audioplayer\|globalconfig\|objectdescriptionmodel\|audiooutputadaptor\|effectwidget\|mediacontrols\|videoplayer\|seekslider\|volumeslider\).cpp$"

if test -n "$1" -a -f "$1"; then
	echo "preprocessing $1"
	cpp $1 2>/dev/null > tmp
	echo "extracting backend calls from $1"
	$EXTRACT tmp > tests/methods/$1
	rm tmp
else
	for i in *.cpp; do
		if echo $i | grep -q "$IGNORE"; then
			printf "%-30s ignored.\n" "$i:"
		else
			printf "%-30s preprocessing" "$i:"
			cpp $i 2>/dev/null > tmp
			echo -n ", extracting backend calls"
			$EXTRACT tmp > tests/methods/$i
			rm tmp
			echo "."
		fi
	done
fi
