#! /usr/bin/env bash
find . -type d | fgrep -v '.svn' | fgrep -v '.pc' | sed -e 's,$,/,' > dirs
msh=`find . -name Messages.sh`
for dir in $msh; do
  dir=`dirname $dir`
  if test "$dir" != "."; then
    egrep -v "^$dir" dirs > dirs.new && mv dirs.new dirs
  fi
done
fgrep -v "/tests" dirs > dirs.new && mv dirs.new dirs
# Include ./kdeui/colors/ explicitly to extract color dialog strings. It is
# otherwise excluded, since it has a local Messages.sh to extract color names.
echo "./kdeui/colors/" >> dirs
dirs=`cat dirs`
find $dirs -maxdepth 1 -name "*.cpp" -print > files
find $dirs -maxdepth 1 -name "*.cc" -print >> files
find $dirs -maxdepth 1 -name "*.h" -print >> files
# Add the kdecore/date files not added in kdecalendarsystems.pot
find ./kdecore/date -name "*.cpp" -o -name "*.cc" -o -name "*.h" | grep -v -E kcalendarsystem.* >> files
$EXTRACTRC `find $dirs -maxdepth 1 \( -name \*.rc -o -name \*.ui -o -name \*.ui3 -o -name \*.ui4 -o -name \*.kcfg \) ` >> rc.cpp || exit 11
$XGETTEXT -ktranslate:1,1t -ktranslate:1c,2,2t --files-from=files -o $podir/kdelibs4.pot
rm -f dirs files
