#!/bin/sh

#
# Prepares the data from testfiles/list for tests in QML in the same way as
# tst_qmimedatabase::findByName_data() for C++.
# For QML, instead of passing the data in QTest columns the data is passed
# via the JavaScript include file testdata_list.js.
#

doit()
{
    SRCDIR=../qmimedatabase/

    prefix=$SRCDIR"testfiles/"

    f=$prefix"list"

    cat $f | while read line; do
        if [ "`echo $line | cut -c3-`" = "" -o "`echo $line | cut -c1`" = "#" ]; then
            true
        else
            set $line
            if [ $# -ge 2 ]; then
                filePath=$1
                mimeType=$2
                xFail=$3
                if [ "$isFirst" != "" ]; then
                    echo -n "               "
                    echo -n ", "
                else
                    isFirst="notIsFirst"
                    echo "function list() {"
                    echo "    return ["
                    echo -n "                 "
                fi
                echo "{ filePath: \"$prefix$filePath\", mimeType: \"$mimeType\", xFail: \"$xFail\" }"
            else
                echo "Illegal line: $line" >&2
            fi
        fi
    done
    echo "           ]"
    echo "}"
}

doit > testdata_list.js
