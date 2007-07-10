#! /usr/bin/env bash
$EXTRACTRC `find . -name "*.ui"` > rc.cpp
$XGETTEXT `find . -name "*.cpp"` -o $podir/knotify4.pot
