INCLUDES = -I$(top_builddir)/arts/flow -I$(top_srcdir)/arts/mcop -I$(top_srcdir)/arts/flow $(all_includes)
LDFLAGS  = $(top_builddir)/arts/mcop/libmcop.la $(top_builddir)/arts/flow/libartsflow.la
CLOSURE = $(libartsmedia_la_OBJECTS) $(libartsmedia_idl_la_OBJECTS)

####### Files

lib_LTLIBRARIES = libartsmedia.la libartsmedia_idl.la

libartsmedia_idl_la_SOURCES = artsmedia.cc
libartsmedia_idl_la_LDFLAGS = -no-undefined

libartsmedia_la_LIBADD = libartsmedia_idl.la
libartsmedia_la_LDFLAGS = -no-undefined
libartsmedia_la_SOURCES = mediaframe.cc

artsincludedir = $(includedir)/arts
artsinclude_HEADERS = mediaframe.h artsmedia.h artsmedia.idl format.h

DISTCLEANFILES = artsmedia.cc artsmedia.h

######## Build rules

artsmedia.cc: artsmedia.h
artsmedia.mcoptype: artsmedia.h
artsmedia.mcopclass: artsmedia.h
artsmedia.h: $(top_srcdir)/arts/media/artsmedia.idl $(MCOPIDL)
	$(MCOPIDL) -I$(top_srcdir)/arts/flow $(top_srcdir)/arts/media/artsmedia.idl
