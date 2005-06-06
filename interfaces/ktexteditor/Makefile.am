INCLUDES = -I$(top_srcdir)/kio -I$(top_srcdir) -I$(top_srcdir)/interfaces -I$(top_builddir)/interfaces  -I$(top_srcdir)/kabc -I$(top_builddir)/kabc $(all_includes)

lib_LTLIBRARIES = libktexteditor.la

libktexteditor_la_SOURCES = ktexteditor.cpp templateinterface.cpp editorchooser.cpp
libktexteditor_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libktexteditor_la_LIBADD = $(LIB_KPARTS) $(top_builddir)/kabc/libkabc.la

ktexteditorinclude_HEADERS = document.h view.h plugin.h cursor.h \
                             editinterface.h \
                             selectioninterface.h cursorinterface.h \
                             menuinterface.h searchinterface.h highlightinginterface.h \
                             codecompletioninterface.h \
                             configinterface.h markinterface.h \
                             encodinginterface.h sessionconfiginterface.h editorchooser.h \
			     texthintinterface.h variableinterface.h templateinterface.h \
			     modificationinterface.h commandinterface.h

ktexteditorincludedir = $(includedir)/ktexteditor

METASOURCES = AUTO

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = ktexteditor.desktop ktexteditorplugin.desktop

kcm_ktexteditor_DATA = kcm_ktexteditor.desktop
kcm_ktexteditordir = $(kde_datadir)/kcm_componentchooser

templateinterface.lo: $(top_builddir)/kabc/addressee.h
