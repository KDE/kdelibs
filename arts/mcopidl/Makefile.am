
INCLUDES = $(all_includes) -I$(srcdir)/..
LDFLAGS   = $(all_libraries)
LDADD = ../mcop/libmcop.la $(LIBSOCKET)

####### Files

bin_PROGRAMS = mcopidl

mcopidl_SOURCES = mcopidl.cc yacc.cc scanner.cc
noinst_HEADERS = yacc.cc.h

####### Build rules

parser:
	cd $(srcdir) && flex -B -8 -oscanner.cc scanner.ll  ;\
	bison -d -t -o yacc.cc yacc.yy

