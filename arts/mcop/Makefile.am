
MCOPIDL = $(top_builddir)/arts/mcopidl/mcopidl
##
lib_LTLIBRARIES = libmcop.la
#
# Programs to compile, Manpages to install and Versions
#
INCLUDES = $(all_includes)
libmcop_la_SOURCES = buffer.cc connection.cc core.cc dispatcher.cc \
     iomanager.cc object.cc socketconnection.cc tcpconnection.cc \
	 unixconnection.cc tcpserver.cc unixserver.cc objectmanager.cc \
	 factory.cc idlfilereg.cc ifacerepo_impl.cc mcoputils.cc \
	 startupmanager.cc md5.c md5auth.cc referenceclean.cc
libmcop_la_LIBADD = $(LIBSOCKET)

artsincludedir = $(includedir)/arts
artsinclude_HEADERS = buffer.h common.h connection.h core.h dispatcher.h \
  factory.h flowsystem.h idlfilereg.h ifacerepo_impl.h iomanager.h \
  mcoputils.h  object.h objectmanager.h pool.h socketconnection.h \
  startupmanager.h tcpconnection.h tcpserver.h type.h unixconnection.h \
  unixserver.h  referenceclean.h

core: $(top_srcdir)/arts/mcop/core.idl $(MCOPIDL)
	$(MCOPIDL) $(top_srcdir)/arts/mcop/core.idl

#
# -frepo closure (experimental)
#
closure: $(libmcop_la_OBJECTS)
	@echo "Building -frepo compliant library closure"
	@echo "int main() {}" > closure.cc
	$(CXXLINK) -o closure $(libmcop_la_OBJECTS) closure.cc
	@echo "closure timestamp" > closure
	@rm -f closure.cc
