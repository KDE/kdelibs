
INCLUDES = $(all_includes)
LDFLAGS   = $(all_libraries)

####### Files

bin_PROGRAMS = dcopidl2cpp

dcopidl2cpp_SOURCES = main.cpp skel.cpp stub.cpp stubimpl.cpp
noinst_HEADERS = main.h

dcopidl2cpp_LDADD = $(LIB_QT)
dcopidl2cpp_LDFLAGS = $(KDE_RPATH)

