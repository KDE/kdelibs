####### General stuff

INCLUDES= -I$(srcdir)/../ $(all_includes)
LDFLAGS	= $(all_libraries)  -version-info 1:0:0
libkparts2_la_LIBADD  = $(LIB_KDEUI)

####### Files

lib_LTLIBRARIES = libkparts2.la

libkparts2_la_SOURCES = kpart.cpp plugin.cpp kembedmanager.cpp

include_HEADERS = kpart.h plugin.h kembedmanager.h

libkparts2_la_METASOURCES = kpart.moc plugin.moc kembedmanager.moc

servicetype_DATA = kpart.desktop
servicetypedir = $(kde_servicetypesdir)

