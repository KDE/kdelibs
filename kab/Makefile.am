INCLUDES= $(all_includes)

noinst_LTLIBRARIES = libkab.la
KABVERSION=	4.0.0
KABPATCH=	0

libkab_la_SOURCES	= \
	kabapi.cc \
	addressbook.cc \
	qconfigDB.cc 

libkab_la_LDFLAGS = $(KDE_MT_LDFLAGS) -version-info 5:0:1 -no-undefined
libkab_la_LIBADD = ../kdeui/libkdeui.la

libkab_la_METASOURCES = AUTO
