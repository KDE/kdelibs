SUBDIRS = . test

INCLUDES = -I$(top_srcdir)/kde3support/kmdi/res -I$(top_srcdir)/kde3support/kdeui -I$(top_srcdir)/kde3support/kparts -I$(top_srcdir)/kutils -I$(top_srcdir) -I$(top_srcdir)/kde3support -I$(top_srcdir)/kde3supprt/kmdi $(all_includes)

noinst_LTLIBRARIES = libk3mdi.la
libk3mdi_la_LDFLAGS = $(all_libraries)
libk3mdi_la_LIBADD  = $(LIB_KPARTS) $(top_builddir)/kutils/libkutils.la

libk3mdi_la_SOURCES = k3mdichildarea.cpp k3mdichildfrm.cpp k3mdichildfrmcaption.cpp \
                     k3mdichildview.cpp k3mdimainfrm.cpp k3mditaskbar.cpp k3mdidockcontainer.cpp \
		     k3mditoolviewaccessor.cpp k3mdiguiclient.cpp k3mdidocumentviewtabwidget.cpp \
                     k3mdifocuslist.cpp

include_HEADERS = k3mdichildarea.h k3mdichildfrm.h k3mdichildfrmcaption.h k3mdidefines.h \
                  k3mdiiterator.h k3mdilistiterator.h k3mdimainfrm.h k3mdinulliterator.h \
                  k3mditaskbar.h k3mdichildview.h k3mditoolviewaccessor.h

METASOURCES = AUTO

include ../../admin/Doxyfile.am
