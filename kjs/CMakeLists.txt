kde4_header()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/global.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/global.h )

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(CREATE_HASH_TABLE ${CMAKE_CURRENT_SOURCE_DIR}/create_hash_table )

macro(CREATE_LUT _in_FILE _out_FILE _dep_FILE)

   add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_out_FILE}
      COMMAND ${PERL_EXECUTABLE}
      ARGS ${CREATE_HASH_TABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${_in_FILE} -i > ${CMAKE_CURRENT_BINARY_DIR}/${_out_FILE}
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_in_FILE} )

endmacro(CREATE_LUT)

create_lut(date_object.cpp date_object.lut.h date_object.cpp)
create_lut(number_object.cpp number_object.lut.h number_object.cpp)
create_lut(string_object.cpp string_object.lut.h string_object.cpp)
create_lut(array_object.cpp array_object.lut.h array_object.cpp)
create_lut(math_object.cpp math_object.lut.h math_object.cpp)
create_lut(keywords.table lexer.lut.h lexer.cpp)

set(kjs_LIB_SRCS
ustring.cpp
date_object.cpp
collector.cpp
nodes.cpp
grammar.cpp
lexer.cpp
lookup.cpp
operations.cpp
regexp.cpp
function_object.cpp
string_object.cpp
bool_object.cpp
number_object.cpp
internal.cpp
array_object.cpp
math_object.cpp
object_object.cpp
regexp_object.cpp
error_object.cpp
function.cpp
debugger.cpp
value.cpp
list.cpp
object.cpp
interpreter.cpp
property_map.cpp
property_slot.cpp
nodes2string.cpp
identifier.cpp
reference.cpp
reference_list.cpp
scope_chain.cpp
dtoa.cpp
protected_values.cpp
fpconst.cpp
)

kde4_automoc(${kjs_LIB_SRCS})

kde4_add_library(kjs SHARED ${kjs_LIB_SRCS})

target_link_libraries(kjs  ${QT_AND_KDECORE_LIBS} m kxmlcore pthread )

set_target_properties(kjs PROPERTIES VERSION 1.2.0 SOVERSION 1 )
install_targets(/lib kjs )


########### next target ###############

set(testkjs_static_SRCS
testkjs.cpp
)

kde4_automoc(${testkjs_static_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(testkjs_static ${testkjs_static_SRCS})

target_link_libraries(testkjs_static  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(testkjs_SRCS
testkjs.cpp
)

kde4_automoc(${testkjs_SRCS})

if(KDE4_BUILD_TESTS)
kde4_add_executable(testkjs ${testkjs_SRCS})

target_link_libraries(testkjs  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_create_doxygen_docs( REFERENCES kdecore  )

kde4_footer()



#original Makefile.am contents follow:

##    This file is part of the KDE libraries
##    Copyright (C) 1999 Harri Porten (porten@kde.org)
#
##    This library is free software; you can redistribute it and/or
##    modify it under the terms of the GNU Lesser General Public
##    License as published by the Free Software Foundation; either
##    version 2 of the License, or (at your option) any later version.
#
##    This library is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
##    Lesser General Public License for more details.
#
##    You should have received a copy of the GNU Lesser General Public License
##    along with this library; see the file COPYING.LIB.  If not, write to
##    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
##    Boston, MA 02110-1301, USA.
#
#YACC = bison
#INCLUDES = $(PCRECFLAGS) $(all_includes)
#
#lib_LTLIBRARIES = libkjs.la
#
#libkjs_la_SOURCES =  ustring.cpp date_object.cpp collector.cpp nodes.cpp \
#		     grammar.cpp lexer.cpp lookup.cpp operations.cpp \
#		     regexp.cpp function_object.cpp string_object.cpp \
#		     bool_object.cpp number_object.cpp internal.cpp \
#		     array_object.cpp math_object.cpp object_object.cpp \
#		     regexp_object.cpp error_object.cpp function.cpp \
#		     debugger.cpp value.cpp list.cpp object.cpp \
#		     interpreter.cpp property_map.cpp property_slot.cpp nodes2string.cpp \
#		     identifier.cpp reference.cpp reference_list.cpp \
#		     scope_chain.cpp dtoa.cpp protected_values.cpp fpconst.cpp
#
#kjsincludedir = $(includedir)/kjs
#kjsinclude_HEADERS = value.h types.h object.h interpreter.h operations.h \
#		     ustring.h lookup.h global.h identifier.h property_map.h \
#		     reference.h reference_list.h completion.h scope_chain.h \
#		     list.h simple_number.h function.h
#
#noinst_HEADERS = internal.h collector.h nodes.h lexer.h lexer.lut.h \
#		 grammar.h regexp.cpp function_object.h string_object.h \
#		 bool_object.h number_object.h date_object.h array_object.h \
#		 math_object.h object_object.h regexp_object.h error_object.h \
#		 debugger.h array_instance.h context.h dtoa.h regexp.h
#
#if include_VERSION_SCRIPT
#VSCRIPT = -Wl,--version-script=$(srcdir)/libkjs.map
#endif
#
#libkjs_la_LDFLAGS = -version-info 3:0:2 -no-undefined $(VSCRIPT) \
#          $(USER_LDFLAGS) $(all_libraries)
## FIXME: remove dependencies and cleanup once we have our own unicode support
#libkjs_la_LIBADD = -lm $(LIBPCRE) ../kxmlcore/libkxmlcore.la -lpthread $(LIB_QT)
#
#EXTRA_DIST = grammar.y
#
#parser: $(srcdir)/grammar.y
#	cd $(srcdir); \
#	$(YACC) -d -p kjsyy grammar.y && mv grammar.tab.c grammar.cpp; \
#	if test -f grammar.tab.h; then \
#	if cmp -s grammar.tab.h grammar.h; then rm -f grammar.tab.h; \
#	else mv grammar.tab.h grammar.h; fi \
#	else :; fi
#
#LUT_FILES = math_object.lut.h lexer.lut.h array_object.lut.h date_object.lut.h string_object.lut.h number_object.lut.h regexp_object.lut.h
#
#CREATE_HASH_TABLE = $(srcdir)/create_hash_table
#
#lexer.lut.h: $(srcdir)/keywords.table $(CREATE_HASH_TABLE)
#	$(PERL) $(CREATE_HASH_TABLE) $(srcdir)/keywords.table -i > $@
#lexer.lo: lexer.lut.h
#
## Can't use %.lut.h: %.cpp, it's not portable.
#
#array_object.lut.h : $(srcdir)/array_object.cpp $(CREATE_HASH_TABLE)
#	$(PERL) $(CREATE_HASH_TABLE) $(srcdir)/array_object.cpp -i > $@
#array_object.lo: array_object.lut.h
#math_object.lut.h : $(srcdir)/math_object.cpp $(CREATE_HASH_TABLE)
#	$(PERL) $(CREATE_HASH_TABLE) $(srcdir)/math_object.cpp -i > $@
#math_object.lo: math_object.lut.h
#date_object.lut.h : $(srcdir)/date_object.cpp $(CREATE_HASH_TABLE)
#	$(PERL) $(CREATE_HASH_TABLE) $(srcdir)/date_object.cpp -i > $@
#date_object.lo: date_object.lut.h
#number_object.lut.h : $(srcdir)/number_object.cpp $(CREATE_HASH_TABLE)
#	$(PERL) $(CREATE_HASH_TABLE) $(srcdir)/number_object.cpp -i > $@
#number_object.lo: number_object.lut.h
#string_object.lut.h : $(srcdir)/string_object.cpp $(CREATE_HASH_TABLE)
#	$(PERL) $(CREATE_HASH_TABLE) $(srcdir)/string_object.cpp -i > $@
#string_object.lo: string_object.lut.h
#regexp_object.lut.h : $(srcdir)/regexp_object.cpp $(CREATE_HASH_TABLE)
#	$(PERL) $(CREATE_HASH_TABLE) $(srcdir)/regexp_object.cpp -i > $@
#regexp_object.lo: regexp_object.lut.h
#
#CLEANFILES = $(LUT_FILES)
#
### test program (in one program for easier profiling/memory debugging)
#EXTRA_PROGRAMS = testkjs_static
#testkjs_static_SOURCES = testkjs.cpp 
#testkjs_static_LDADD = $(LIBPCRE) libkjs.la
#testkjs_static_LDFLAGS = -static
#
### test program (linked to libkjs)
#check_PROGRAMS = testkjs
#testkjs_SOURCES = testkjs.cpp
#testkjs_LDADD = libkjs.la
#
#DOXYGEN_REFERENCES = kdecore
#include ../admin/Doxyfile.am
#
#.PHONY: parser
#
