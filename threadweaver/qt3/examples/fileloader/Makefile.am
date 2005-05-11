# KDE Data Distribution Service Daemon and Tools
#    (threader / threads / jobs example)
#
# (C) 2003 by Mirko Boehm
# email: mirko@kde.org
# www.hackerbuero.org

INCLUDES= -I../../weaver -I../../weaverextensions -I../../jobs $(all_includes)

bin_PROGRAMS = fileloader

METASOURCES = AUTO

# noinst_HEADERS =

fileloader_SOURCES = imageview_base.ui imageview.cpp

fileloader_LDFLAGS = $(KDE_MT_LDFLAGS) $(all_libraries) $(KDE_RPATH)

fileloader_LDADD = -lssl ../../weaver/libweaver.la \
	../../weaverextensions/libweaverextensions.la \
	../../jobs/libjobs.la \
	$(LIB_KDEUI) $(LIB_QT) 

# manual dependancy:
# gui.h : gui_base.h
