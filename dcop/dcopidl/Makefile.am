
INCLUDES = $(all_includes)
AM_LDFLAGS   = $(all_libraries)
AM_CPPFLAGS = -DYYDEBUG

####### Files

bin_PROGRAMS = dcopidl

dcopidl_SOURCES = main.cpp yacc.cc scanner.cc
noinst_HEADERS = yacc.cc.h

dcopidl_LDADD = $(LIB_QT)
dcopidl_LDFLAGS = $(KDE_RPATH)

####### Build rules

parser:
	cd $(srcdir) && flex -B -8 -oscanner.cc scanner.ll  ;\
	bison -t -d -v -o yacc.cc yacc.yy && mv yacc.hh yacc.cc.h

$(srcdir)/dcopidl_output.kidl: dcopidl $(srcdir)/dcopidl_test.h
	builddir=`pwd`; cd $(srcdir) ; $$builddir/dcopidl ./dcopidl_test.h > dcopidl_output.kidl

check-local: $(srcdir)/dcopidl_output.kidl

EXTRA_DIST = dcopidl_test.h scanner.ll yacc.yy

MAINTAINERCLEANFILES = yacc.cc.output
