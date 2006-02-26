#!/bin/sh

# Regression testing: generate .kidl out of dcopidl_test.h and compare with expected baseline
# Usage: $srcdir/run_test.sh $srcdir

srcdir="$1"
builddir=`pwd`
# Make a symlink in dcopidlng's builddir, to have "./dcopidl_test.h" in the kidl
rm -f dcopidlng_test.h ; ln -s $srcdir/dcopidl_test.h dcopidlng_test.h
# Note that dcopidlng might not be installed yet, so we can't use the dcopidlng script
# (which looks into kde's datadir)
dcopidlng="perl -I$srcdir $srcdir/kalyptus --allow_k_dcop_accessors -f dcopidl"
$dcopidlng ./dcopidlng_test.h > $builddir/dcopidl_new_output.kidl || exit 1
diff -u $srcdir/dcopidl_output.kidl $builddir/dcopidl_new_output.kidl
