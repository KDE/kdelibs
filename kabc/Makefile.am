SUBDIRS = vcard . formats plugins

INCLUDES = -I$(srcdir)/vcard/include -I$(srcdir)/vcard/include/generated \
  -I$(top_srcdir)/kabc -I$(top_srcdir)/kab $(all_includes)

lib_LTLIBRARIES   = libkabc.la
libkabc_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 2:0:1 -module
libkabc_la_LIBADD  = vcard/libvcard.la ../kdeui/libkdeui.la ../kio/libkio.la

libkabc_la_SOURCES = \
  address.cpp addressbook.cpp addressee.cpp addresseedialog.cpp \
  distributionlist.cpp distributionlisteditor.cpp errorhandler.cpp \
  field.cpp formatfactory.cpp geo.cpp key.cpp phonenumber.cpp plugin.cpp \
  resource.cpp resourceconfigwidget.cpp resourcefactory.cpp resourcefile.cpp \
  resourcefileconfig.cpp resourceselectdialog.cpp secrecy.cpp \
  stdaddressbook.cpp timezone.cpp vcard21parser.cpp vcardconverter.cpp \
  vcardformat.cpp vcardformatimpl.cpp


kabcincludedir = $(includedir)/kabc
kabcinclude_HEADERS = address.h addressbook.h addressee.h addresseedialog.h \
  distributionlist.h distributionlisteditor.h errorhandler.h field.h \
  formatfactory.h geo.h kabcformat.h key.h phonenumber.h plugin.h resource.h \
  resourceconfigwidget.h resourcefactory.h resourcefile.h \
  resourceselectdialog.h secrecy.h stdaddressbook.h timezone.h \
  vcard21parser.h vcardconverter.h vcardformat.h vcardformatimpl.h

METASOURCES = AUTO

bin_PROGRAMS = kab2kabc

kab2kabc_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kab2kabc_LDADD = libkabc.la ../kab/libkab.la
kab2kabc_SOURCES = kab2kabc.cpp

check_PROGRAMS = testkabc testkabcdlg testdistlist bigread bigwrite testdb \
  testaddressee kabcargl

testkabc_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testkabc_LDADD = libkabc.la
testkabc_SOURCES = testkabc.cpp

testaddressee_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testaddressee_LDADD = libkabc.la
testaddressee_SOURCES = testaddressee.cpp

testkabcdlg_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testkabcdlg_LDADD = libkabc.la
testkabcdlg_SOURCES = testkabcdlg.cpp

testdistlist_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testdistlist_LDADD = libkabc.la
testdistlist_SOURCES = testdistlist.cpp

bigread_LDFLAGS = $(all_libraries) $(KDE_RPATH)
bigread_LDADD = libkabc.la
bigread_SOURCES = bigread.cpp

bigwrite_LDFLAGS = $(all_libraries) $(KDE_RPATH)
bigwrite_LDADD = libkabc.la
bigwrite_SOURCES = bigwrite.cpp

testdb_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testdb_LDADD = libkabc.la
testdb_SOURCES = testdb.cpp 

kabcargl_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kabcargl_LDADD = libkabc.la
kabcargl_SOURCES = kabcargl.cpp

autostart_DATA = kab2kabc.desktop
autostartdir = $(prefix)/share/autostart

DOXYGEN_REFERENCES = kdecore kdeui

kde_module_LTLIBRARIES = kcm_kabc.la

kcm_kabc_la_SOURCES = kcmkabc.cpp resourceconfigdlg.cpp
kcm_kabc_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kcm_kabc_la_LIBADD = $(LIB_KIO) libkabc.la

data_DATA = kabc.desktop
datadir = $(kde_appsdir)/Settings/Personalization

EXTRA_DIST = $(data_DATA)

include ../Doxyfile.am
