INCLUDES= -I$(srcdir)/../.. -I$(srcdir)/.. $(all_includes)

####### Files

kde_module_LTLIBRARIES = kio_ftp.la

kio_ftp_la_SOURCES = ftp.cc
kio_ftp_la_LIBADD  = ../libksycoca.la
kio_ftp_la_LDFLAGS = -module $(KDE_PLUGIN)

noinst_HEADERS = ftp.h

kdelnk_DATA = ftp.protocol
kdelnkdir = $(kde_servicesdir)


