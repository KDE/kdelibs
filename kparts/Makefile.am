####### General stuff

INCLUDES= -I$(srcdir)/../ -I$(top_srcdir)/kio/ -I$(top_srcdir)/libltdl \
	-I$(top_srcdir)/kfile $(all_includes)
libkparts_la_LDFLAGS = $(KDE_MT_LDFLAGS) -version-info 3:0:1 -no-undefined
libkparts_la_LIBADD  = $(LIB_KIO)

check-local:
	(cd tests && $(MAKE) check )

####### Files

lib_LTLIBRARIES = libkparts.la

libkparts_la_SOURCES = part.cpp plugin.cpp partmanager.cpp mainwindow.cpp dockmainwindow.cpp \
		       event.cpp browserextension.cpp factory.cpp  historyprovider.cpp \
		       browserinterface.cpp browserrun.cpp

kpartsincludedir = $(includedir)/kparts
kpartsinclude_HEADERS = part.h plugin.h partmanager.h mainwindow.h dockmainwindow.h event.h \
	browserextension.h factory.h historyprovider.h browserinterface.h genericfactory.h \
	componentfactory.h browserrun.h

libkparts_la_METASOURCES = AUTO

servicetype_DATA = kpart.desktop krop.desktop krwp.desktop browserview.desktop 
servicetypedir = $(kde_servicetypesdir)

DOXYGEN_REFERENCES = kdecore kdeui kdefx kio
include ../admin/Doxyfile.am
