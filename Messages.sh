#! /usr/bin/env bash
### TODO: would it not be easier to use -j for xgettext from a copy of qt-messages.pot ?
cd kabc/scripts && perl ./makeaddressee
find . -type d | fgrep -v '.svn' | sed -e 's,$,/,' > dirs
mfs=`find . -name Makefile.am | xargs egrep -l '^messages:'`
msh=`find . -name Messages.sh`
for dir in $mfs $msh; do
  if test "$dir" != "./Makefile.am"; then
    dir=`dirname $dir`
    egrep -v "^$dir" dirs > dirs.new && mv dirs.new dirs
  fi
done
fgrep -v "/tests" dirs > dirs.new && mv dirs.new dirs
dirs=`sort -u dirs`
find $dirs -maxdepth 1 -name "*.cpp" -print > files
find $dirs -maxdepth 1 -name "*.cc" -print >> files
find $dirs -maxdepth 1 -name "*.h" -print >> files
echo ./kdecore/kde-config.cpp.in >> files
$EXTRACTRC `find $dirs -maxdepth 1 \( -name \*.rc -o -name \*.ui -o -name \*.ui3 -o -name \*.ui4 -o -name \*.kcfg \) ` >> rc.cpp || exit 11
: > kde.pot
$XGETTEXT -ktranslate:1,1t -ktranslate:1c,2,2t -x qt-messages.pot --files-from=files && cat messages.po qt-messages.pot > $podir/kdelibs.pot
$XGETTEXT common_texts.cpp -o kde.pot
rm -f dirs
