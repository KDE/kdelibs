lib_LTLIBRARIES = libjobs.la

INCLUDES= -I../weaver $(all_includes)

include_HEADERS = fileloaderjob.h qimageloaderjob.h

# noinst_HEADERS = 

libjobs_la_SOURCES = fileloaderjob.cpp qimageloaderjob.cpp

libjobs_la_LDFLAGS = $(KDE_MT_LDFLAGS) $(QT_LDFLAGS) -version-info 0:0

# libweaver_la_LIBADD = 

libjob_la_METASOURCES = AUTO

