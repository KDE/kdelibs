#!/bin/sh

# Regression testing: generate .kidl out of dcopidl_test.h and compare with expected baseline
# Usage: $srcdir/run_test.sh $srcdir

srcdir="$1"
builddir=`pwd`
cd $srcdir
"$builddir/dcopidl" ./dcopidl_test.h > $builddir/dcopidl_new_output.kidl || exit 1
diff -u $srcdir/dcopidl_output.kidl $builddir/dcopidl_new_output.kidl
