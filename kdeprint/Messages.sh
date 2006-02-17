#! /usr/bin/env bash
$EXTRACTRC `find . -name "*.rc" -o -name "*.ui" | grep -v /cupsdconf` >> rc.cpp || exit 11
$XGETTEXT `find . -name "*.cpp" | grep -v /cupsdconf` -o $podir/kdeprint.pot
