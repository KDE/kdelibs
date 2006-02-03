#!/bin/sh

perl ./generate.pl < testcases || exit 1

qmake dcop_test.pro -o Makefile && $MAKE || exit 1
qmake driver.pro -o Makefile && $MAKE || exit 1
