CXXFLAGS = $(NOOPT_CXXFLAGS) $(USE_RTTI)
INCLUDES = -I$(srcdir)/../mcop $(all_includes)
LDFLAGS   = $(all_libraries)
LDADD = $(USE_THREADS) ../mcop/libmcop.la

####### Files

bin_PROGRAMS = mcopidl

mcopidl_SOURCES = mcopidl.cc yacc.cc scanner.cc namespace.cc
noinst_HEADERS = yacc.cc.h

####### Build rules

parser:
	cd $(srcdir) && flex -B -8 -oscanner.cc scanner.ll  ;\
	bison -d -t -o yacc.cc yacc.yy

