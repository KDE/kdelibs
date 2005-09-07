SUBDIRS = .

INCLUDES = -I$(top_srcdir)/kutils -I$(top_srcdir) -I$(top_srcdir)/kde3support/kparts -I$(top_srcdir)/kde3support/kdeui $(all_includes)

noinst_LTLIBRARIES = libk3parts.la
libk3parts_la_LDFLAGS = $(all_libraries)
libk3parts_la_LIBADD  = $(LIB_KPARTS)

libk3parts_la_SOURCES = dockmainwindow3.cpp

include_HEADERS = dockmainwindow3.h

METASOURCES = AUTO

include ../../admin/Doxyfile.am
