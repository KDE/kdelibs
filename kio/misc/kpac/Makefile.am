
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libltdl $(all_includes)

kde_module_LTLIBRARIES = libkpac.la
bin_PROGRAMS = kpac_dhcp_helper

libkpac_la_SOURCES = kproxybindings.cpp kpac_impl.cpp kpac_downloader.cpp \
    kpac_discovery.cpp

libkpac_la_LIBADD = $(top_builddir)/kio/libkio.la $(top_builddir)/kjs/libkjs.la
libkpac_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

noinst_HEADERS = kproxybindings.h kpac_impl.h kpac_downloader.h \
    kpac_discovery.h dhcp.h

METASOURCES = AUTO

kpac_dhcp_helper_SOURCES = kpac_dhcp_helper.c
kpac_dhcp_helper_LDADD = $(LIBSOCKET)

install-exec-local:
	@(chown root $(bindir)/kpac_dhcp_helper && chmod 4755 $(bindir)/kpac_dhcp_helper) || echo "Please make kpac_dhcp_helper setuid root"
