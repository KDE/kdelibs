INCLUDES= $(all_includes) 

lib_LTLIBRARIES = libkab.la
KABVERSION=	4.0.0
KABPATCH=	0

include_HEADERS = \
	kabapi.h \
	addressbook.h \
	qconfigDB.h 

libkab_la_SOURCES	= \
	kabapi.cc \
	addressbook.cc \
	qconfigDB.cc 

libkab_la_LDFLAGS = -version-info 4:0 -no-undefined
libkab_la_LIBADD = $(LIB_KDEUI) 

libkab_la_METASOURCES = AUTO

picsdir = $(kde_datadir)/kab/pics
pics_DATA = 

templatedir = $(kde_datadir)/kab
template_DATA = template.kab template.config
