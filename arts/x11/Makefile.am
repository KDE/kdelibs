INCLUDES = -I$(top_srcdir)/arts/mcop $(all_includes)

lib_LTLIBRARIES = libx11globalcomm.la

libx11globalcomm_la_SOURCES = x11globalcomm.cc x11globalcomm_impl.cc
libx11globalcomm_la_LIBADD = $(top_builddir)/arts/mcop/libmcop.la $(LIB_X11)
libx11globalcomm_la_LDFLAGS = -no-undefined -module -version-info 1:0 $(X_LDFLAGS)

DISTCLEANFILES = x11globalcomm.cc x11globalcomm.h \
                 x11globalcomm.mcoptype x11globalcomm.mcopclass

x11globalcomm.h: $(srcdir)/x11globalcomm.idl $(MCOPIDL)
	$(MCOPIDL) -t $(INCLUDES) $(srcdir)/x11globalcomm.idl

x11globalcomm.cc: x11globalcomm.h
x11globalcomm_impl.cc: x11globalcomm.h
x11globalcomm.mcoptype: x11globalcomm.h
x11globalcomm.mcopclass: x11globalcomm.h

mcopclassdir = $(libdir)/mcop/Arts
mcopclass_DATA = X11GlobalComm.mcopclass

######## install idl typeinfo files

mcoptypedir = $(libdir)/mcop
mcoptype_DATA = x11globalcomm.mcoptype x11globalcomm.mcopclass
