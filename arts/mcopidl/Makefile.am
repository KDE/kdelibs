INCLUDES = $(all_includes) -I$(top_srcdir)/arts/mcop
LDFLAGS   = $(all_libraries) $(top_builddir)/arts/mcop/libmcop.la

####### Files

bin_PROGRAMS = mcopidl

mcopidl_SOURCES = mcopidl.cc yacc.cc scanner.cc
noinst_HEADERS = yacc.cc.h

DISTCLEANFILES = $(SRCMETA)

####### Implicit rules

####### Build rules

parser:
	cd $(srcdir) && flex -B -8 -oscanner.cc scanner.ll  ;\
	bison -d -t -o yacc.cc yacc.yy

