# KDE Data Distribution Service Daemon and Tools
#    (threader / threads / jobs example)
#
# (C) 2003 by Mirko Boehm
# email: mirko@kde.org
# www.hackerbuero.org

INCLUDES= -I../../weaver -I../../weaverextensions $(all_includes)

bin_PROGRAMS = requests

METASOURCES = AUTO

noinst_HEADERS = requests.h

requests_SOURCES = processrequest.ui requests.cpp

requests_LDFLAGS = $(KDE_MT_LDFLAGS) $(all_libraries) $(KDE_RPATH)

requests_LDADD = -lssl ../../weaver/libweaver.la ../../weaverextensions/libweaverextensions.la \
	$(LIB_KDEUI) $(LIB_QT) 

# manual dependancy:
# requests.cpp : processrequest.h
