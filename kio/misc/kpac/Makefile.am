
kde_module_LTLIBRARIES = kded_proxyscout.la
bin_PROGRAMS = kpac_dhcp_helper

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libltdl -I$(top_srcdir)/kdecore/network -I$(top_srcdir)/kded -I$(top_builddir)/kjs \
           -I$(top_srcdir)/kdecore/network $(all_includes)
METASOURCES = AUTO
KDE_CXXFLAGS = $(USE_EXCEPTIONS)

kded_proxyscout_la_SOURCES = proxyscout.skel proxyscout.cpp script.cpp \
                             downloader.cpp discovery.cpp
kded_proxyscout_la_LDFLAGS = $(all_libraries) -module -avoid-version
kded_proxyscout_la_LIBADD = $(LIB_KIO) $(top_builddir)/kjs/libkjs.la $(LIB_KDED)

kpac_dhcp_helper_SOURCES = kpac_dhcp_helper.c
kpac_dhcp_helper_CFLAGS = $(KDE_USE_FPIE)
kpac_dhcp_helper_LDFLAGS = $(KDE_USE_PIE)
kpac_dhcp_helper_LDADD = $(LIBSOCKET)

noinst_HEADERS = proxyscout.h script.h downloader.h discovery.h

servicesdir = $(kde_servicesdir)/kded
services_DATA = proxyscout.desktop

proxyscoutdatadir = $(kde_datadir)/proxyscout
proxyscoutdata_DATA = eventsrc

install-exec-local:
	@(chown root $(DESTDIR)$(bindir)/kpac_dhcp_helper && chmod 4755 $(DESTDIR)$(bindir)/kpac_dhcp_helper) || echo "Please make kpac_dhcp_helper setuid root"
