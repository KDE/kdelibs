
INCLUDES = $(all_includes)
LDFLAGS   = $(all_libraries)

####### Files

bin_PROGRAMS = dcop dcopfind dcopclient dcopobject dcopref

noinst_HEADERS =

dcop_SOURCES = dcop.cpp
dcop_LDADD = $(LIB_QT) $(LIB_DCOP)
dcop_LDFLAGS = $(KDE_RPATH)

dcopfind_SOURCES = dcopfind.cpp
dcopfind_LDADD = $(LIB_QT) $(LIB_DCOP)
dcopfind_LDFLAGS = $(KDE_RPATH)

dcopclient_SOURCES = dcopclient.c
dcopobject_SOURCES = dcopobject.c
dcopref_SOURCES = dcopref.c
