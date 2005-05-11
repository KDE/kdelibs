# KDE Data Distribution Service Daemon and Tools
#    (threader / threads / jobs example)
#
# (C) 2003 by Mirko Boehm
# email: mirko@kde.org
# www.hackerbuero.org

INCLUDES= -I../../weaver -I../../weaverextensions $(all_includes)

bin_PROGRAMS = jobs-gui

METASOURCES = AUTO

noinst_HEADERS = gui.h

jobs_gui_SOURCES = gui_base.ui gui.cpp

jobs_gui_LDFLAGS = $(KDE_MT_LDFLAGS) $(all_libraries) $(KDE_RPATH)

jobs_gui_LDADD = -lssl ../../weaver/libweaver.la ../../weaverextensions/libweaverextensions.la \
	$(LIB_KDEUI) $(LIB_QT) 

# manual dependancy:
gui.h : gui_base.h
