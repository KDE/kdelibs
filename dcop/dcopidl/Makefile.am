
INCLUDES = $(all_includes)
LDFLAGS   = $(all_libraries)

####### Files

bin_PROGRAMS = kidl

kidl_SOURCES = main.cpp yacc.cc scanner.cc
noinst_HEADERS = yacc.cc.h

kidl_LDADD = $(LIB_QT)
kidl_LDFLAGS = $(KDE_RPATH)

DISTCLEANFILES = $(SRCMETA)

####### Implicit rules

####### Build rules

parser:
	cd $(srcdir) && flex -B -8 -oscanner.cc scanner.ll  ;\
	bison -d -t -v -o yacc.cc yacc.yy

