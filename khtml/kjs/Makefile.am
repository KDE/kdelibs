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
INCLUDES = -I$(top_srcdir)/khtml $(all_includes)

lib_LTLIBRARIES = libkjs.la

libkjs_la_SOURCES =  kjs.cpp grammar.cpp lexer.cpp nodes.cpp object.cpp \
	operations.cpp property.cpp kjsstring.cpp math_object.cpp \
	html_object.cpp bool_object.cpp object_object.cpp array_object.cpp

include_HEADERS = kjs.h

noinst_HEADERS = global.h object.h nodes.h operations.h lexer.h \
	kjsstring.h grammar.h object_object.h function_object.h \
	bool_object.h math_object.h html_object.h array_object.h

libkjs_la_LDFLAGS = -version-info 1:0 $(QT_LDFLAGS)
libkjs_la_LIBADD = $(LIB_KDECORE) -lm

parser: $(srcdir)/grammar.y
	cd $(srcdir); \
	$(YACC) -d -p kjsyy grammar.y && mv grammar.tab.c grammar.cpp; \
	if test -f grammar.tab.h; then \
	if cmp -s grammar.tab.h grammar.h; then rm -f grammar.tab.h; \
	else mv grammar.tab.h grammar.h; fi \
	else :; fi


## test program
check_PROGRAMS = testkjs
testkjs_SOURCES = testkjs.cpp $(libkjs_la_SOURCES)
testkjs_LDADD = $(libkjs_la_LIBADD) $(LIB_KHTML)
