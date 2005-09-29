
INCLUDES = $(all_includes)
KDE_CXXFLAGS = -UQT3_SUPPORT
AM_LDFLAGS   = $(all_libraries)
AM_CPPFLAGS = -DYYDEBUG

####### Files

bin_PROGRAMS = dcopidlold

dcopidlold_SOURCES = main.cpp yacc.cc scanner.cc
noinst_HEADERS = yacc.cc.h

dcopidlold_LDADD = $(LIB_QT)
dcopidlold_LDFLAGS = $(KDE_RPATH)

####### Build rules

parser:
	cd $(srcdir) && flex -B -8 -oscanner.cc scanner.ll  ;\
	bison -t -d -v -o yacc.cc yacc.yy && mv yacc.hh yacc.cc.h

check-local: 
	$(srcdir)/run_test.sh $(srcdir)

EXTRA_DIST = dcopidl_test.h scanner.ll yacc.yy

MAINTAINERCLEANFILES = yacc.cc.output
