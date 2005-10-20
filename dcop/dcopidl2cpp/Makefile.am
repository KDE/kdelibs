
INCLUDES = $(all_includes)
KDE_CXXFLAGS = -UQT3_SUPPORT
AM_LDFLAGS   = $(all_libraries)

####### Files

bin_PROGRAMS = dcopidl2cpp

dcopidl2cpp_SOURCES = main.cpp skel.cpp stub.cpp stubimpl.cpp
noinst_HEADERS = main.h

dcopidl2cpp_LDADD = $(LIB_QT) $(LIB_QTXML)
dcopidl2cpp_LDFLAGS = $(KDE_RPATH)

# Run the command with ./dcopidl_test.h so that this is what gets into dcopidl_output.kidl
# (no full path, even with srcdir!=builddir)
dcopidl_output.kidl: $(DCOPIDL) $(srcdir)/dcopidl_test.h
	bd=`pwd`; cd $(srcdir) && $(DCOPIDL) ./dcopidl_test.h > $$bd/dcopidl_output.kidl

check-local: dcopidl_output.kidl
	./dcopidl2cpp dcopidl_output.kidl
