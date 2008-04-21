
INCLUDES = $(all_includes)
AM_LDFLAGS   = $(all_libraries)

####### Files

bin_PROGRAMS = dcopidl2cpp

dcopidl2cpp_SOURCES = main.cpp skel.cpp stub.cpp stubimpl.cpp
noinst_HEADERS = main.h

dcopidl2cpp_LDADD = $(LIB_QT)
dcopidl2cpp_LDFLAGS = $(KDE_RPATH) $(AM_LDFLAGS)

dcopidl_output.kidl: ../dcopidl/dcopidl $(srcdir)/dcopidl_test.h
	../dcopidl/dcopidl $(srcdir)/dcopidl_test.h > dcopidl_output.kidl

check-local: dcopidl_output.kidl
	./dcopidl2cpp dcopidl_output.kidl
	$(MD5SUM) dcopidl_output_*
