####### General stuff

INCLUDES= -I$(srcdir)/../ -I$(top_srcdir)/kio/ -I$(top_srcdir)/libltdl \
$(all_includes)
LDFLAGS	= $(all_libraries)  -version-info 1:0:0
libkparts_la_LIBADD  = $(LIB_KIO)

SUBDIRS = . tests

####### Files

lib_LTLIBRARIES = libkparts.la

libkparts_la_SOURCES = kpart.cpp kplugin.cpp kpartmanager.cpp kpartsmainwindow.cpp kxmlgui.cpp

include_HEADERS = kpart.h kplugin.h kpartmanager.h kpartsmainwindow.h kxmlgui.h

libkparts_la_METASOURCES = AUTO

servicetype_DATA = kpart.desktop
servicetypedir = $(kde_servicetypesdir)

