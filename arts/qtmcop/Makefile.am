lib_LTLIBRARIES = libqtmcop.la

INCLUDES = -I$(top_srcdir)/arts/mcop $(all_includes)
KDE_OPTIONS = qtonly

libqtmcop_la_SOURCES = qiomanager.cc
libqtmcop_la_LIBADD = $(top_builddir)/arts/mcop/libmcop.la $(LIB_QT)
libqtmcop_la_LDFLAGS = -no-undefined -version-info 1:0 $(QT_LDFLAGS) \
  $(X_LDFLAGS) $(USER_LDFLAGS)

METASOURCES = qiomanager_p.moc

artsincludedir = $(includedir)/arts
artsinclude_HEADERS = qiomanager.h
