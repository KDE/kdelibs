INCLUDES = $(all_includes)

METASOURCES = AUTO

lib_LTLIBRARIES = libkresources.la

libkresources_la_SOURCES = resource.cpp \
  factory.cpp \
  manageriface.skel \
  manageriface.stub \
  managerimpl.cpp \
  configwidget.cpp \
  configdialog.cpp \
  selectdialog.cpp \
  configpage.cpp

libkresources_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 3:0:2
libkresources_la_LIBADD = $(LIB_KDEUI) $(LIB_KIO)

kresincludedir = $(includedir)/kresources
kresinclude_HEADERS = resource.h \
  configwidget.h \
  factory.h \
  manageriface.h \
  managerimpl.h \
  manager.h \
  selectdialog.h \
  configdialog.h \
  configpage.h 

check_PROGRAMS = testresources

testresources_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testresources_LDADD   = libkresources.la
testresources_SOURCES = testresources.cpp

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = kresources_plugin.desktop
