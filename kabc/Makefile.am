SUBDIRS = vcard vcardparser . formats plugins scripts

# Make sure $(all_includes) remains last!
INCLUDES = -I$(top_builddir)/kabc -I$(top_srcdir)/kabc -I$(top_srcdir)/kab \
           -I$(srcdir)/vcardparser/ -I$(srcdir)/vcard/include \
           -I$(srcdir)/vcard/include/generated \
           -I$(srcdir)/vcardparser $(all_includes)

addressee.h addressee.cpp field.cpp: \
				$(srcdir)/scripts/makeaddressee \
				$(srcdir)/scripts/addressee.src.cpp \
				$(srcdir)/scripts/addressee.src.h \
				$(srcdir)/scripts/entrylist \
				$(srcdir)/scripts/field.src.cpp
	mysrcdir=`cd $(srcdir)/scripts && pwd` ;\
	cd scripts && $(PERL) $$mysrcdir/makeaddressee

CLEANFILES = addressee.h addressee.cpp field.cpp

lib_LTLIBRARIES = libkabc.la
libkabc_la_COMPILE_FIRST = addressee.h
libkabc_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 3:0:2 -no-undefined
libkabc_la_LIBADD = vcard/libvcard.la vcardparser/libvcards.la $(LIB_KIO) \
                    $(top_builddir)/kresources/libkresources.la

libkabc_la_SOURCES = \
  address.cpp addressbook.cpp addressee.cpp addresseedialog.cpp agent.cpp \
  distributionlist.cpp distributionlistdialog.cpp distributionlisteditor.cpp \
  errorhandler.cpp field.cpp formatfactory.cpp geo.cpp key.cpp \
  phonenumber.cpp picture.cpp plugin.cpp resource.cpp \
  resourceselectdialog.cpp secrecy.cpp sound.cpp stdaddressbook.cpp \
  timezone.cpp vcard21parser.cpp vcardconverter.cpp vcardformat.cpp \
  vcardformatimpl.cpp vcardformatplugin.cpp ldifconverter.cpp addresslineedit.cpp \
  ldapclient.cpp addresseelist.cpp vcardtool.cpp addresseehelper.cpp \
  addresseehelper.skel lock.cpp locknull.cpp ldif.cpp ldapurl.cpp ldapconfigwidget.cpp


kabcincludedir = $(includedir)/kabc
kabcinclude_HEADERS = address.h addressbook.h addressee.h addresseedialog.h \
  agent.h distributionlist.h distributionlistdialog.h distributionlisteditor.h \
  errorhandler.h field.h format.h formatfactory.h formatplugin.h geo.h key.h \
  phonenumber.h picture.h plugin.h resource.h \
  resourceselectdialog.h secrecy.h sound.h stdaddressbook.h timezone.h \
  vcardconverter.h vcardformat.h vcardformatplugin.h ldifconverter.h \
  addresslineedit.h ldapclient.h addresseelist.h lock.h locknull.h ldif.h \
  ldapurl.h ldapconfigwidget.h

METASOURCES = AUTO

bin_PROGRAMS = kab2kabc

kab2kabc_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kab2kabc_LDADD = libkabc.la ../kab/libkab.la
kab2kabc_SOURCES = kab2kabc.cpp

check_PROGRAMS = testlock

testlock_LDFPLAGS = $(all_libraries)
testlock_LDADD = libkabc.la
testlock_SOURCES = testlock.cpp

EXTRA_PROGRAMS = testkabc testkabcdlg testdistlist bigread bigwrite testdb \
  testaddressee testaddresseelist testaddressfmt kabcargl testaddresslineedit

testkabc_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testkabc_LDADD = libkabc.la
testkabc_SOURCES = testkabc.cpp

testaddressee_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testaddressee_LDADD = libkabc.la
testaddressee_SOURCES = testaddressee.cpp

testaddresseelist_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testaddresseelist_LDADD = libkabc.la
testaddresseelist_SOURCES = testaddresseelist.cpp

testaddressfmt_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testaddressfmt_LDADD = libkabc.la
testaddressfmt_SOURCES = testaddressfmt.cpp

testkabcdlg_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testkabcdlg_LDADD = libkabc.la
testkabcdlg_SOURCES = testkabcdlg.cpp

testdistlist_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testdistlist_LDADD = libkabc.la
testdistlist_SOURCES = testdistlist.cpp

testaddresslineedit_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testaddresslineedit_LDADD = libkabc.la
testaddresslineedit_SOURCES = testaddresslineedit.cpp

bigread_LDFLAGS = $(all_libraries) $(KDE_RPATH)
bigread_LDADD = libkabc.la $(top_builddir)/kabc/plugins/file/libkabc_file.la
bigread_SOURCES = bigread.cpp

bigwrite_LDFLAGS = $(all_libraries) $(KDE_RPATH)
bigwrite_LDADD = libkabc.la $(top_builddir)/kabc/plugins/file/libkabc_file.la
bigwrite_SOURCES = bigwrite.cpp

testdb_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testdb_LDADD = libkabc.la
testdb_SOURCES = testdb.cpp 

kabcargl_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kabcargl_LDADD = libkabc.la
kabcargl_SOURCES = kabcargl.cpp

autostart_DATA = kab2kabc.desktop
autostartdir = $(datadir)/autostart

DOXYGEN_REFERENCES = kdecore kdeui

map_DATA = countrytransl.map
mapdir = $(kde_datadir)/kabc

EXTRA_DIST = $(map_DATA)

include ../admin/Doxyfile.am
