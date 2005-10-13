SUBDIRS = . ui tests plugins kcmspellchecking

METASOURCES = AUTO

noinst_LTLIBRARIES = libkspell2base.la

AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -I$(top_srcdir)/kutils $(all_includes)

kspell2baseincludedir = $(includedir)/kspell2

kspell2baseinclude_HEADERS = settings.h filter.h \
	dictionary.h broker.h \
	backgroundchecker.h defaultdictionary.h 

libkspell2base_la_SOURCES = settings.cpp broker.cpp \
	client.cpp filter.cpp backgroundchecker.cpp \
	backgroundthread.cpp backgroundengine.cpp \
	defaultdictionary.cpp 

libkspell2base_la_LDFLAGS = -no-undefined $(all_libraries)
libkspell2base_la_LIBADD  = $(LIB_KPARTS) ../kutils/libkutils.la 

servicetype_DATA = kspellclient.desktop
servicetypedir = $(kde_servicetypesdir)

include $(top_srcdir)/admin/Doxyfile.am
