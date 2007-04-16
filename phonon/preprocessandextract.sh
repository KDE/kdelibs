#!/bin/sh
EXTRACT="`dirname $0`/extractmethodcalls.rb"
IGNORE="^\(guiimpl\|kiofallbackimpl\|kiofallback\|guiinterface\|deinterlacefilter\|audiodevice\|audiodeviceenumerator\|audioplayer\|globalconfig\|objectdescriptionmodel\|audiooutputadaptor\|effectwidget\|mediacontrols\|videoplayer\|seekslider\|volumeslider\).cpp$"

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
		elif echo $i | grep -q '_p\.cpp$'; then
			printf "%-30s postponed.\n" "$i:"
		else
			printf "%-30s preprocessing" "$i:"
			cpp $i 2>/dev/null > tmp
			echo -n ", extracting backend calls"
			$EXTRACT tmp > tests/methods/$i
			rm tmp
			echo "."
		fi
	done
	for i in *_p.cpp; do
		cpp=`echo $i | sed 's,_p\.cpp$,\.cpp,'`
		if echo $cpp | grep -q "$IGNORE"; then
			printf "%-30s ignored.\n" "$i:"
		elif test "$i" != "*_p.cpp"; then
			printf "%-30s preprocessing" "$i:"
			cpp $i 2>/dev/null > tmp
			echo -n ", extracting backend calls"
			$EXTRACT tmp >> tests/methods/$cpp
			rm tmp
			echo "."
		fi
	done
fi
