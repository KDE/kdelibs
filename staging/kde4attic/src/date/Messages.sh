#!/bin/bash
$XGETTEXT $(find -name "kcalendarsystem*.cpp") -o $podir/kdecalendarsystems.pot
$XGETTEXT TIMEZONES -o $podir/timezones4.pot
