
AM_CXXFLAGS = -DEXTENSION_DIR='"$(libdir)"' -DTRADER_DIR='"$(libdir)/mcop"'
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
	 trader_impl.cc dynamicrequest.cc anyref.cc loopback.cc debug.cc \
	 delayedreturn.cc thread.cc dynamicskeleton.cc

libmcop_la_LIBADD = $(LIBSOCKET) $(top_builddir)/libltdl/libltdlc.la
libmcop_la_LDFLAGS = -no-undefined -version-info 1:0

artsincludedir = $(includedir)/arts
artsinclude_HEADERS = buffer.h common.h connection.h core.h dispatcher.h \
  factory.h flowsystem.h idlfilereg.h ifacerepo_impl.h iomanager.h \
  mcoputils.h  object.h objectmanager.h pool.h socketconnection.h \
  startupmanager.h tcpconnection.h tcpserver.h type.h unixconnection.h \
  unixserver.h  referenceclean.h datapacket.h asyncstream.h notification.h \
  extensionloader.h mcopconfig.h connect.h reference.h weakreference.h \
  dynamicrequest.h anyref.h debug.h delayedreturn.h dynamicskeleton.h thread.h \
  core.idl

core: $(srcdir)/core.idl $(MCOPIDL)
	$(MCOPIDL) -e Arts::Object -I$(srcdir) $(srcdir)/core.idl
