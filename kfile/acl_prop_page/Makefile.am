# $Id$


INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kio -I$(top_builddir)/kio $(srcdir)/.. $(all_includes)

EXTRA_LTLIBRARIES = libkacl_prop_page.la

lib_LTLIBRARIES = $(KFILE_ACL_MODULE)

libkacl_prop_page_la_LDFLAGS = -module $(KDE_PLUGIN)
libkacl_prop_page_la_LIBADD = $(LIB_KFILE) $(LIBPOSIX1E)
libkacl_prop_page_la_SOURCES = acl.cpp kaclmanipulation.cpp

noinst_HEADERS = acl.h kaclmanipulation.h

service_DATA = kaclplugin.desktop
servicedir = $(kde_servicesdir)

METASOURCES = AUTO

