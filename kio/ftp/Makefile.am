INCLUDES= -I$(srcdir)/../.. -I$(srcdir)/.. $(all_includes)

####### Files

lib_LTLIBRARIES = kio_ftp.la # kio_ftpproxy.la

kio_ftp_la_SOURCES = ftp.cc
kio_ftp_la_LIBADD  = ../libkio.la 
kio_ftp_la_LDFLAGS = -module -avoid-version -no-undefined

# kio_ftpproxy_la_SOURCES = ftp_proxy.cc
# kio_ftpproxy_la_LIBADD  = ../libkio.la ../http/kio_http.la
# kio_ftpproxy_la_LDFLAGS = -module -avoid-version -no-undefined

noinst_HEADERS = ftp.h

kioconfig_DATA = ftp.desktop
kioconfigdir = $(kde_confdir)/protocols


