#    This file is part of the KDE libraries
#    Copyright (C) 1999 Harri Porten (porten@kde.org)

#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.

#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.

#    You should have received a copy of the GNU Library General Public License
#    along with this library; see the file COPYING.LIB.  If not, write to
#    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
#    Boston, MA 02111-1307, USA.

YACC = bison
INCLUDES = $(PCRECFLAGS) $(all_includes)

lib_LTLIBRARIES = libkjs.la

libkjs_la_SOURCES =  ustring.cpp internal.cpp collector.cpp nodes.cpp \
		     grammar.cpp lexer.cpp lookup.cpp operations.cpp \
		     regexp.cpp function_object.cpp string_object.cpp \
		     bool_object.cpp number_object.cpp date_object.cpp \
		     array_object.cpp math_object.cpp object_object.cpp \
		     regexp_object.cpp error_object.cpp function.cpp \
		     debugger.cpp value.cpp types.cpp object.cpp \
		     interpreter.cpp property_map.cpp nodes2string.cpp

kjsincludedir = $(includedir)/kjs
kjsinclude_HEADERS = value.h types.h object.h interpreter.h operations.h \
		     ustring.h

noinst_HEADERS = internal.h collector.h nodes.h lexer.h lexer.lut.h lookup.h \
		 grammar.h regexp.cpp function_object.h string_object.h \
		 bool_object.h number_object.h date_object.h array_object.h \
		 math_object.h object_object.h regexp_object.h error_object.h \
		 function.h debugger.h property_map.h

libkjs_la_LDFLAGS = -version-info 1:0 -no-undefined $(USER_LDFLAGS) $(all_libraries)
libkjs_la_LIBADD = -lm $(LIBPCRE)

parser: $(srcdir)/grammar.y
	cd $(srcdir); \
	$(YACC) -d -p kjsyy grammar.y && mv grammar.tab.c grammar.cpp; \
	if test -f grammar.tab.h; then \
	if cmp -s grammar.tab.h grammar.h; then rm -f grammar.tab.h; \
	else mv grammar.tab.h grammar.h; fi \
	else :; fi

hashtables:
	cd $(srcdir); \
	./create_hash_table keywords.table -i > lexer.lut.h; \
	./create_hash_table array_object.cpp -i > array_object.lut.h; \
	./create_hash_table math_object.cpp -i > math_object.lut.h; \
	./create_hash_table date_object.cpp -i > date_object.lut.h; \
	./create_hash_table number_object.cpp -i > number_object.lut.h; \
	./create_hash_table string_object.cpp -i > string_object.lut.h

## test program (in one program for easier profiling/memory debugging)
EXTRA_PROGRAMS = testkjs_static testavl
testkjs_static_SOURCES = testkjs.cpp 
testkjs_static_LDADD = $(LIBPCRE) libkjs.la
testkjs_static_LDFLAGS = -static
testavl_SOURCES = testavl.cpp 
testavl_LDADD = $(LIBPCRE) libkjs.la

## test program (linked to libkjs)
check_PROGRAMS = testkjs
testkjs_SOURCES = testkjs.cpp
testkjs_LDADD = libkjs.la

DOXYGEN_REFERENCES = kdecore
