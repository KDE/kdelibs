#!/bin/sh

if test -z "$1"; then
	echo "Usage: $0 <classname>"
	exit 1
fi
CLASSNAME=$1
FILENAME=`echo $CLASSNAME|tr '[:upper:]' '[:lower:]'`
HEADER="${FILENAME}_methodtest.h"
IMPL="${FILENAME}_methodtest.cpp"

if test -e $HEADER -o -e $IMPL; then
	echo "Test already exists, please remove first"
	exit 1
fi

#selection=foo
#while test -n $selection; do
#	i=0
#	for file in method_*.cpp; do
#		echo "[`printf %02d $i`] $file"
#		i=$(($i+1))
#	done
#
#	echo select one:
#	read selection
#done

cat test_template.h   | sed "s/Template/$CLASSNAME/g" > $HEADER
cat test_template.cpp | sed "s/Template/$CLASSNAME/g" | sed "s/test_template/${FILENAME}_methodtest/" | sed "s/REPLACE_WITH_INCLUDES/#include \"methods\/${FILENAME}.cpp\"/" > $IMPL

cat CMakeLists.txt | sed "s/PHONON_DECLARE_TESTS(/PHONON_DECLARE_TESTS(\n\t${FILENAME}_methodtest/" > tmp;
mv tmp CMakeLists.txt
