SUBDIRS = ui . tests plugins
METASOURCES = AUTO

lib_LTLIBRARIES = libkspell2.la

AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -I$(top_srcdir)/kutils $(all_includes)

kspell2includedir = $(includedir)/kspell2

kspell2include_HEADERS = settings.h filter.h \
	dictionary.h broker.h \
	backgroundchecker.h defaultdictionary.h 

libkspell2_la_SOURCES = settings.cpp broker.cpp \
	client.cpp filter.cpp backgroundchecker.cpp \
	backgroundthread.cpp backgroundengine.cpp \
	defaultdictionary.cpp 

libkspell2_la_LDFLAGS = -no-undefined -version-info 1:0:0 $(all_libraries)
libkspell2_la_LIBADD  = $(LIB_KPARTS) ../kutils/libkutils.la ./ui/libkspell2ui.la 

servicetype_DATA = kspellclient.desktop
servicetypedir = $(kde_servicetypesdir)
KDE_OPTIONS = nofinal
