#!/bin/bash

qmake kded.pro && nmake -nologo clean && nmake -nologo || exit 1
qmake kded_rel.pro && nmake -nologo clean && nmake -nologo || exit 1
