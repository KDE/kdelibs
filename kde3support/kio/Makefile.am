INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kio/bookmarks -I$(top_srcdir)/kde3support/kdecore $(all_includes)

noinst_LTLIBRARIES = libk3io.la
libk3io_la_LDFLAGS = $(all_libraries)
libk3io_la_LIBADD  = $(LIB_KPARTS)

libk3io_la_SOURCES = k3bookmarkdrag.cpp

include_HEADERS = k3bookmarkdrag.h

METASOURCES = AUTO

include ../../admin/Doxyfile.am
