# $Id: Makefile.am 334039 2004-07-30 03:27:07Z nhasan $

INCLUDES= -I$(srcdir)/.. $(all_includes) 

####### Files

kde_module_LTLIBRARIES = kded_test.la

kded_test_la_SOURCES = test.cpp test.skel
kded_test_la_METASOURCES = AUTO
kded_test_la_LDFLAGS = $(all_libraries) -module -avoid-version
kded_test_la_LIBADD = $(LIB_KDECORE)

servicesdir = $(kde_servicesdir)/kded
services_DATA = test.desktop

