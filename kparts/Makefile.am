####### General stuff

INCLUDES= -I$(srcdir)/../ $(all_includes)
LDFLAGS	= $(all_libraries)  -version-info 1:0:0
libkparts2_la_LIBADD  = $(LIB_KIO)

####### Files

lib_LTLIBRARIES = libkparts2.la

libkparts2_la_SOURCES = kpart.cpp kplugin.cpp kpartmanager.cpp kshell.cpp kxmlgui.cpp

include_HEADERS = kpart.h kplugin.h kpartmanager.h kshell.h kxmlgui.h

libkparts2_la_METASOURCES = AUTO

servicetype_DATA = kpart.desktop
servicetypedir = $(kde_servicetypesdir)

