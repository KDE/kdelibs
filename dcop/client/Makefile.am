
INCLUDES = $(all_includes)
DCOP_LIB = ../libDCOP.la

####### Files

bin_PROGRAMS = dcop dcopfind dcopclient dcopobject dcopref dcopstart

dcop_SOURCES = dcop.cpp
dcop_LDADD = $(LIB_QT) $(DCOP_LIB)
dcop_LDFLAGS = $(KDE_RPATH) $(all_libraries)

dcopstart_SOURCES = dcopstart.cpp
dcopstart_LDADD = $(LIB_QT) $(DCOP_LIB)
dcopstart_LDFLAGS = $(KDE_RPATH) $(all_libraries)

dcopfind_SOURCES = dcopfind.cpp
dcopfind_LDADD = $(LIB_QT) $(DCOP_LIB)
dcopfind_LDFLAGS = $(KDE_RPATH) $(all_libraries)

dcopclient_SOURCES = dcopclient.c
dcopclient_LDFLAGS = $(all_libraries)
dcopobject_SOURCES = dcopobject.c
dcopobject_LDFLAGS= $(all_libraries)
dcopref_SOURCES = dcopref.c
dcopref_LDFLAGS = $(all_libraries)
