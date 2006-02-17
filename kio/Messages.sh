#! /usr/bin/env bash
$EXTRACTRC `find . ../kioslave -name \*.rc -o -name \*.ui -o -name \*.ui3 -o -name \*.ui4` >> rc.cpp || exit 11
$XGETTEXT `find . ../kioslave -name "*.cpp" -o -name "*.cc" -o -name "*.h"` -o $podir/kio.pot
