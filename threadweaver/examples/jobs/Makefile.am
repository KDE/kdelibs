# KDE Data Distribution Service Daemon and Tools
#    (weaver / threads / jobs example)
#
# (C) 2003 by Mirko Boehm
# email: mirko@kde.org
# www.hackerbuero.org

INCLUDES= $(all_includes) -I../../weaver -I../../weaverextensions

bin_PROGRAMS = jobs

METASOURCES = AUTO

noinst_HEADERS =

jobs_SOURCES = jobs.cpp

jobs_LDFLAGS = $(KDE_MT_LDFLAGS) $(all_libraries) $(KDE_RPATH)

jobs_LDADD = -lssl ../../weaver/libweaver.la \
	../../weaverextensions/libweaverextensions.la \
	$(LIB_KDEUI) $(LIB_QT) 

