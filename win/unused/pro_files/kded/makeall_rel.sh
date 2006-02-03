#!/bin/bash

qmake kded.pro KW_CONFIG=release -o Makefile.release && nmake -nologo -f Makefile.release clean && nmake -nologo -f Makefile.release || exit 1
qmake kded_rel.pro KW_CONFIG=release -o Makefile.release && nmake -nologo -f Makefile.release clean && nmake -nologo -f Makefile.release || exit 1
