#!/bin/sh

# usage: findnewjobs <old-cupd-conf> <new-cupsd-conf>

if [ $# -ne "2" ]; then
	echo "Wrong argument number: findnewjobs <old> <new>"
	exit -1
fi

NEWCUPS="$2"
OLDCUPS="$1"

OPTSNEW=`mktemp /tmp/cupsdconf.XXXXXX`
OPTSOLD=`mktemp /tmp/cupsdconf.XXXXXX`

grep -e '^#\?[a-zA-Z0-9]' $NEWCUPS | awk '{print $1}' | sed 's,#,,' | sort | uniq > $OPTSNEW
grep -e '^#\?[a-zA-Z0-9]' $OLDCUPS | awk '{print $1}' | sed 's,#,,' | sort | uniq > $OPTSOLD

echo " "
echo "Added options"
echo "-------------"
diff $OPTSOLD $OPTSNEW | grep '^> ' | sed 's,> ,,'

echo " "
echo "Removed options"
echo "--------------"
diff $OPTSOLD $OPTSNEW | grep '^< ' | sed 's,< ,,'

rm -f $OPTSOLD $OPTSNEW
