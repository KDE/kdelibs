SUBDIRS = kab vcardparser . formats plugins scripts tests

# Make sure $(all_includes) remains last!
INCLUDES = -I$(top_builddir)/kabc -I$(top_srcdir)/kabc -I$(top_srcdir)/kabc/kab \
           -I$(srcdir)/vcardparser $(all_includes)

field.cpp: addressee.h addressee.cpp
addressee.cpp: addressee.h
addressee.cpp addressee.h field.cpp: \
				$(srcdir)/scripts/makeaddressee \
				$(srcdir)/scripts/addressee.src.cpp \
				$(srcdir)/scripts/addressee.src.h \
				$(srcdir)/scripts/entrylist \
				$(srcdir)/scripts/field.src.cpp
	mysrcdir=`cd $(srcdir)/scripts && pwd` ;\
	cd scripts && $(PERL) $$mysrcdir/makeaddressee

CLEANFILES = addressee.h addressee.cpp field.cpp

lib_LTLIBRARIES = libkabc.la
libkabc_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 3:0:2 -no-undefined
libkabc_la_LIBADD = vcardparser/libvcards.la $(LIB_KIO) \
                    $(top_builddir)/kresources/libkresources.la
libkabc_la_COMPILE_FIRST = addressee.h

libkabc_la_SOURCES = \
  address.cpp addressbook.cpp addressee.cpp addresseedialog.cpp \
  distributionlist.cpp distributionlistdialog.cpp distributionlisteditor.cpp \
  errorhandler.cpp field.cpp formatfactory.cpp geo.cpp key.cpp \
  phonenumber.cpp picture.cpp plugin.cpp resource.cpp \
  secrecy.cpp sound.cpp stdaddressbook.cpp \
  vcardconverter.cpp timezone.cpp \
  vcardformat.cpp ldifconverter.cpp addresslineedit.cpp \
  ldapclient.cpp addresseelist.cpp vcardtool.cpp addresseehelper.cpp \
  addresseehelper.skel lock.cpp locknull.cpp ldif.cpp ldapurl.cpp ldapconfigwidget.cpp \
  sortmode.cpp


kabcincludedir = $(includedir)/kabc
kabcinclude_HEADERS = address.h addressbook.h addressee.h addresseedialog.h \
  distributionlist.h distributionlistdialog.h distributionlisteditor.h \
  errorhandler.h field.h format.h formatfactory.h format.h geo.h key.h \
  phonenumber.h picture.h plugin.h resource.h \
  secrecy.h sound.h stdaddressbook.h timezone.h \
  vcardformat.h vcardconverter.h ldifconverter.h \
  addresslineedit.h ldapclient.h addresseelist.h lock.h locknull.h ldif.h \
  ldapurl.h ldapconfigwidget.h sortmode.h

METASOURCES = AUTO

bin_PROGRAMS = kab2kabc

kab2kabc_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kab2kabc_LDADD = libkabc.la ./kab/libkab.la
kab2kabc_SOURCES = kab2kabc.cpp

autostart_DATA = kab2kabc.desktop
autostartdir = $(datadir)/autostart

manager_DATA = kabc_manager.desktop
managerdir = $(kde_servicesdir)/kresources

DOXYGEN_REFERENCES = kdecore kdeui
DOXYGEN_EXCLUDE = scripts

map_DATA = countrytransl.map
mapdir = $(kde_datadir)/kabc

EXTRA_DIST = $(map_DATA)

include ../admin/Doxyfile.am
