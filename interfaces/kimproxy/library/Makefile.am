SUBDIRS = icons

INCLUDES = $(all_includes)

noinst_HEADERS = kimproxyiface.h

METASOURCES = AUTO

AM_CPPFLAGS = $(all_includes)

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kimproxy.pot

kimproxyincludedir = $(includedir)
kimproxyinclude_HEADERS = kimproxy.h kimproxyiface.h

lib_LTLIBRARIES =  libkimproxy.la
libkimproxy_la_LDFLAGS = -no-undefined $(all_libraries)
libkimproxy_la_LIBADD = $(LIB_KIO)
libkimproxy_la_SOURCES = kimproxy.cpp kimiface.stub kimproxyiface.skel

kimiface_DIR = $(top_srcdir)/interfaces/kimproxy/interface

include $(top_srcdir)/admin/Doxyfile.am
DOXYGEN_REFERENCES = kabc dcop

