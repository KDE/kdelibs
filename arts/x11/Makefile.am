INCLUDES = $(all_includes) -I$(top_srcdir)/arts/mcop

lib_LTLIBRARIES = libx11globalcomm.la

libx11globalcomm_la_SOURCES = x11globalcomm.cc x11globalcomm_impl.cc
libx11globalcomm_la_LIBADD = $(top_builddir)/arts/mcop/libmcop.la $(LIB_X11)
libx11globalcomm_la_LDFLAGS = -no-undefined -module $(X_LDFLAGS)

x11globalcomm.h: $(srcdir)/x11globalcomm.idl $(MCOPIDL)
	$(MCOPIDL) $(INCLUDES) $(srcdir)/x11globalcomm.idl

x11globalcomm.cc: x11globalcomm.h

mcopclassdir = $(libdir)/Arts
mcopclass_DATA = X11GlobalComm.mcopclass
