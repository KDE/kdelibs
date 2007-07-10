#! /usr/bin/env bash
find -name "*.cpp" -print > files
find -name "*.cc" -print >> files
find -name "*.h" -print >> files
$XGETTEXT --files-from=files -o $podir/kstyle_config.pot
rm -f files
