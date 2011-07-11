#!/bin/bash

grep KDE_EXPORT *.h | sed 's/:.*KDE_EXPORT//' | sed 's/:.*//g' | sed 's/\([^ ]*\) \(.*\)/echo "#include \\"..\/..\/kactivities\/\1\\"" > includes\/Activities\/\2/g'

