INCLUDES = -I$(top_srcdir)/kdeprint -I$(top_srcdir) $(all_includes)

bin_PROGRAMS = kprinter

kprinter_SOURCES = main.cpp
kprinter_LDADD = $(top_builddir)/kdeprint/libkdeprint.la
kprinter_LDFLAGS = $(all_libraries)
