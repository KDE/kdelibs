#! /usr/bin/env bash
: > colors.cpp
list=`egrep '^[ ]{0,2}[0-9]' rgb.txt  | tr '	' ' ' | sed -e "s#^[ 0-9]*[ 0-9]*[ 0-9]* *##" | fgrep -v " " | fgrep -v gray | fgrep -v grey | LC_ALL=C sort -u`
for i in $list; do
	echo "i18nc(\"color\", \"$i\");" | sed -e "s#!# #g" >> colors.cpp
done
$XGETTEXT colors.cpp -o $podir/kdelibs_colors4.pot
rm -f colors.cpp
