INCLUDES = -I$(top_srcdir)/interfaces/kregexpeditor $(all_includes)

lib_LTLIBRARIES   = libkutils.la
libkutils_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 3:0:2
# Needs KTrader for kregexpeditor
libkutils_la_LIBADD  = ../kio/libkio.la

libkutils_la_SOURCES = kfind.cpp kfinddialog.cpp kreplace.cpp kreplacedialog.cpp \
                       kcmoduleinfo.cpp kcmoduleloader.cpp kcmultidialog.cpp kmultitabbar.cpp \
                       kcddispatcher.cpp kconfiguredialog.cpp kplugininfo.cpp kpluginselector.cpp

include_HEADERS = kfind.h kfinddialog.h kreplace.h kreplacedialog.h \
                  kcmoduleinfo.h kcmoduleloader.h kcmultidialog.h kmultitabbar.h \
                  kcddispatcher.h kconfiguredialog.h kplugininfo.h kpluginselector.h

noinst_HEADERS = kmultitabbar_p.h

METASOURCES = AUTO

DOXYGEN_REFERENCES = kdecore kdeui kio
include ../admin/Doxyfile.am

# vim: et
