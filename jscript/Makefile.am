#    This file is part of the KDE libraries
#    Copyright (C) 1997 Torben Weis (weis@kde.org)
#              (C) 1997 Stephan Kulow (coolo@kde.org)

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


####### General stuff

INCLUDES	= $(QT_INCLUDES) $(X_INCLUDES)
lib_LTLIBRARIES = libjscript.la
YACC		= bison -y
LEX		= flex
YFLAGS 		= -d -p yyjscript
SOMAJOR 	= 2
SOMINOR 	= 0

####### Files

libjscript_la_SOURCES =	t1.c t2.c cpp2bison.cpp jstree.cpp jsexec.cpp builtin.cpp
libjscript_la_LDFLAGS = -version-info $(SOMINOR):$(SOMAJOR) 
libjscript_la_LIBADD =  

include_HEADERS = bison2cpp.h jstree.h jsexec.h jserror.h builtin.h
noinst_HEADERS = t1.h

grammarfiles: t1.y t2.l
	$(YACC) $(YFLAGS) t1.y $< && mv y.tab.c t1.c
	if test -f y.tab.h; then \
	if cmp -s y.tab.h t1.h; then rm -f y.tab.h; \
	else mv y.tab.h t1.h; fi \
	else :; fi
	$(LEX) -Pyyjscript t2.l && mv lex.yyjscript.c t2.c

