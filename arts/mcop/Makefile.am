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
	 startupmanager.cc

core: $(top_srcdir)/arts/mcop/core.idl $(MCOPIDL)
	$(MCOPIDL) $(top_srcdir)/arts/mcop/core.idl
