
INCLUDES = $(all_includes)
AM_LDFLAGS   = $(all_libraries)
DCOP_LIB = ../libDCOP.la

####### Files

bin_PROGRAMS = dcop dcopfind dcopclient dcopobject dcopref dcopstart

dcop_SOURCES = dcop.cpp
dcop_LDADD = $(LIB_QT) $(DCOP_LIB)
dcop_LDFLAGS = $(KDE_RPATH)

dcopstart_SOURCES = dcopstart.cpp
dcopstart_LDADD = $(LIB_QT) $(DCOP_LIB)
dcopstart_LDFLAGS = $(KDE_RPATH)

dcopfind_SOURCES = dcopfind.cpp
dcopfind_LDADD = $(LIB_QT) $(DCOP_LIB)
dcopfind_LDFLAGS = $(KDE_RPATH)

dcopclient_SOURCES = dcopclient.c
dcopobject_SOURCES = dcopobject.c
dcopref_SOURCES = dcopref.c
