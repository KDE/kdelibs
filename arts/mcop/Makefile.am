
MCOPIDL = $(top_builddir)/arts/mcopidl/mcopidl
AM_CXXFLAGS = -DEXTENSION_DIR='"$(libdir)"'
##
lib_LTLIBRARIES = libmcop.la
#
# Programs to compile, Manpages to install and Versions
#
INCLUDES = -I$(top_srcdir)/libltdl $(all_includes)
libmcop_la_SOURCES = buffer.cc connection.cc core.cc dispatcher.cc \
     iomanager.cc object.cc socketconnection.cc tcpconnection.cc \
	 unixconnection.cc tcpserver.cc unixserver.cc objectmanager.cc \
	 factory.cc idlfilereg.cc ifacerepo_impl.cc mcoputils.cc \
	 startupmanager.cc md5.c md5auth.cc referenceclean.cc datapacket.cc \
	 asyncstream.cc notification.cc flowsystem.cc extensionloader.cc \
	 tmpglobalcomm.cc mcopconfig.cc connect.cc reference.cc type.cc \
	 trader_impl.cc dynamicrequest.cc anyref.cc loopback.cc

libmcop_la_LIBADD = $(LIBSOCKET) $(LIBPTHREAD) \
     $(top_builddir)/libltdl/libltdlc.la
libmcop_la_LDFLAGS = -no-undefined 

artsincludedir = $(includedir)/arts
artsinclude_HEADERS = buffer.h common.h connection.h core.h dispatcher.h \
  factory.h flowsystem.h idlfilereg.h ifacerepo_impl.h iomanager.h \
  mcoputils.h  object.h objectmanager.h pool.h socketconnection.h \
  startupmanager.h tcpconnection.h tcpserver.h type.h unixconnection.h \
  unixserver.h  referenceclean.h datapacket.h asyncstream.h notification.h \
  extensionloader.h mcopconfig.h connect.h reference.h weakreference.h

core: $(srcdir)/core.idl $(MCOPIDL)
	$(MCOPIDL) -I$(srcdir) $(srcdir)/core.idl
