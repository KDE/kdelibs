
INCLUDES = $(all_includes)
KDE_CXXFLAGS = -UQT3_SUPPORT
AM_LDFLAGS   = $(all_libraries)
DCOP_LIB = ../libDCOP.la

####### Files

bin_PROGRAMS = dcop dcopfind dcopclient dcopobject dcopref dcopstart dcopquit

dcop_SOURCES = dcop.cpp
dcop_LDADD = $(LIB_QT) $(DCOP_LIB)
dcop_LDFLAGS = $(KDE_RPATH)

dcopstart_SOURCES = dcopstart.cpp
dcopstart_LDADD = $(LIB_QT) $(DCOP_LIB)
dcopstart_LDFLAGS = $(KDE_RPATH)

dcopquit_SOURCES = dcopquit.cpp
dcopquit_LDADD = $(LIB_QT) $(DCOP_LIB)
dcopquit_LDFLAGS = $(KDE_RPATH)

dcopfind_SOURCES = dcopfind.cpp
dcopfind_LDADD = $(LIB_QT) $(DCOP_LIB)
dcopfind_LDFLAGS = $(KDE_RPATH)

dcopclient_SOURCES = dcopclient.c
dcopobject_SOURCES = dcopobject.c
dcopref_SOURCES = dcopref.c
