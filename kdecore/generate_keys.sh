#!/bin/sh

## This script extracts the key names from the QT header
##
## Copyright (c) 1999 Nicolas HADACEK (hadacek@kde.org)
## Distributed under the GNU General Public License

# remove files
rm -f kckey.h kckey.cpp kckey_a

# extract key names and code from QT header
sed -n '/enum Key/!b2
		:1 N
		/}/!b1
		p	
		:2' $1 | sed -n '/=/p' | sed -n '
		s/	Key_/{ "/
		s/,\(.*\)$/ },/
		s/ = /", /p' > kckey_a

# write header file
begin_line="// This file has been automatically genrated by \"generate_keys.sh\"\n\n"
echo -e $begin_line \
		"typedef struct {\n" \
		"\tconst char *name;\n" \
		"\tint code;\n" \
		"} KKeys;\n\n" \
		"#define NB_KEYS " `cat kckey_a | wc -l` "\n" \
		"extern const KKeys KKEYS[NB_KEYS];\n" > kckey.h

# write source file
echo -e $begin_line \
		"#include <qnamespace.h>\n" \
		"#include \"kckey.h\"\n\n" \
		"#define Key_Space Qt::Key_Space // hack around\n" \
		"const KKeys KKEYS[NB_KEYS] = {" > kckey.cpp

cat kckey_a >> kckey.cpp
				
echo -e "} };" >> kckey.cpp

# cleaning
rm -f kckey_a
