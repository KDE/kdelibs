#!/bin/bash

qmake kded.pro && nmake -nologo clean && nmake -nologo || exit 1
qmake kded_gui.pro && nmake -nologo clean && nmake -nologo || exit 1
