INCLUDES = -I$(top_builddir)/kdedb $(all_includes)

METASOURCES = AUTO


lib_LTLIBRARIES = libkdbcore.la

libkdbcore_la_SOURCES = capability.cpp connection.cpp connector.cpp database.cpp dataobject.cpp \
	dbengine.cpp field.cpp index.cpp object.cpp plugin.cpp query.cpp record.cpp \
	recordset.cpp table.cpp recordsetiterator.cpp value.cpp

libkdbcore_la_LDFLAGS = -version-info 0:1:0 -no-undefined

libkdbcore_la_LIBADD = $(LIB_KSYCOCA) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT)


include_HEADERS = capability.h connection.h connector.h database.h dataobject.h dbengine.h \
	exception.h field.h index.h kdb.h object.h plugin.h query.h record.h recordset.h \
	table.h recordsetiterator.h handler.h value.h
includedir = $(kde_includes)/kdb

servicetypesdir = $(kde_servicetypesdir)
servicetypes_DATA = kdbplugin.desktop 
