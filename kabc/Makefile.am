SUBDIRS = vcard

INCLUDES = -I$(srcdir)/vcard/include -I$(srcdir)/vcard/include/generated -I$(top_srcdir)/kab \
  $(all_includes)

lib_LTLIBRARIES   = libkabc.la
libkabc_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 1:0:0 -module
libkabc_la_LIBADD  = $(top_builddir)/kabc/vcard/libvcard.la $(LIB_KDEUI)

libkabc_la_SOURCES = \
  addressbook.cpp addressee.cpp phonenumber.cpp simpleformat.cpp \
  stdaddressbook.cpp vcardformat.cpp address.cpp geo.cpp timezone.cpp \
  vcardformatimpl.cpp addresseedialog.cpp distributionlist.cpp \
  distributionlisteditor.cpp resource.cpp resourcefile.cpp \
  vcard21parser.cpp field.cpp

kabcincludedir = $(includedir)/kabc
kabcinclude_HEADERS = address.h addressbook.h addressee.h addresseedialog.h \
  distributionlist.h distributionlisteditor.h format.h geo.h phonenumber.h \
  simpleformat.h stdaddressbook.h timezone.h vcardformat.h resource.h \
  resourcefile.h vcard21parser.h field.h

METASOURCES = AUTO

bin_PROGRAMS = kab2kabc

kab2kabc_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kab2kabc_LDADD = libkabc.la ../kab/libkab.la
kab2kabc_SOURCES = kab2kabc.cpp

check_PROGRAMS = testkabc testkabcdlg testdistlist bigread bigwrite testdb kabcargl

testkabc_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testkabc_LDADD = libkabc.la
testkabc_SOURCES = testkabc.cpp

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
testdb_SOURCES = testdb.cpp resourcesql.cpp

kabcargl_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kabcargl_LDADD = libkabc.la
kabcargl_SOURCES = kabcargl.cpp

autostart_DATA = kab2kabc.desktop
autostartdir = $(prefix)/share/autostart

DOXYGEN_REFERENCES = kdecore kdeui
