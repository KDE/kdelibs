
INCLUDES = $(all_includes)
LDFLAGS   = $(all_libraries)
AM_CPPFLAGS = -DYYDEBUG

####### Files

bin_PROGRAMS = dcopidl

dcopidl_SOURCES = main.cpp yacc.cc scanner.cc
noinst_HEADERS = yacc.cc.h

dcopidl_LDADD = $(LIB_QT)
dcopidl_LDFLAGS = $(KDE_RPATH)

DISTCLEANFILES = $(SRCMETA)

####### Implicit rules

####### Build rules

parser:
	cd $(srcdir) && flex -B -8 -oscanner.cc scanner.ll  ;\
	bison -t -d -v -o yacc.cc yacc.yy

