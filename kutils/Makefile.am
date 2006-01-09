SUBDIRS = ksettings . tests
INCLUDES = -I$(top_srcdir)/interfaces/kregexpeditor $(all_includes)

## TODO
## KDE_CXXFLAGS = -UQT3_SUPPORT

lib_LTLIBRARIES   = libkutils.la
libkutils_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 3:0:2
# Needs KTrader
libkutils_la_LIBADD  = ../kio/libkio.la ksettings/libksettings.la

libkutils_la_SOURCES = kfind.cpp kfinddialog.cpp kreplace.cpp \
                       kreplacedialog.cpp kcmoduleinfo.cpp kcmoduleloader.cpp \
                       kcmultidialog.cpp kmultitabbar.cpp kplugininfo.cpp \
                       kcmoduleproxy.cpp kpluginselector.cpp \
                       kcmodulecontainer.cpp kcmoduleproxyIface.skel \
                       kcmoduleproxyIfaceImpl.cpp

include_HEADERS = kfind.h kfinddialog.h kreplace.h kreplacedialog.h \
                  kcmoduleinfo.h kcmoduleloader.h kcmultidialog.h \
                  kmultitabbar.h kplugininfo.h kcmoduleproxy.h \
                  kpluginselector.h kcmodulecontainer.h

noinst_HEADERS = kmultitabbar_p.h kpluginselector_p.h \
                 kcmoduleproxyIface.h kcmoduleproxyIfaceImpl.h

kde_servicetypes_DATA = kplugininfo.desktop

METASOURCES = AUTO

DOXYGEN_REFERENCES = kdecore kdeui kio
include ../admin/Doxyfile.am

# vim: et
