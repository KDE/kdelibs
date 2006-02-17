#! /usr/bin/env bash
: > colors.cpp
list=`grep '^[0-9]' rgb.txt  | tr '	' ' ' | sed -e "s#^[0-9]* [0-9]* [0-9]* *##" | fgrep -v " " | fgrep -v gray | fgrep -v grey | LC_ALL=C sort -u`
for i in $list; do
	echo "i18n(\"color\", \"$i\");" | sed -e "s#!# #g" >> colors.cpp
done
$XGETTEXT colors.cpp rc.cpp -o $podir/kdelibs_colors.pot
rm -f colors.cpp
