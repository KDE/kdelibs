SUBDIRS = kmdi . test

INCLUDES = -I$(top_srcdir)/kmdi/res -I$(top_srcdir)/kutils -I$(top_srcdir) -I$(top_srcdir)/kmdi $(all_includes)

lib_LTLIBRARIES = libkmdi.la
libkmdi_la_LDFLAGS = $(all_libraries) -version-info 1:0
libkmdi_la_LIBADD  = $(LIB_KPARTS) $(top_builddir)/kutils/libkutils.la

libkmdi_la_SOURCES = kmdichildarea.cpp kmdichildfrm.cpp kmdichildfrmcaption.cpp \
                     kmdichildview.cpp kmdimainfrm.cpp kmditaskbar.cpp kmdidockcontainer.cpp \
		     kmditoolviewaccessor.cpp kmdiguiclient.cpp kmdidocumentviewtabwidget.cpp \
                     kmdifocuslist.cpp

include_HEADERS = kmdichildarea.h kmdichildfrm.h kmdichildfrmcaption.h kmdidefines.h \
                  kmdiiterator.h kmdilistiterator.h kmdimainfrm.h kmdinulliterator.h \
                  kmditaskbar.h kmdichildview.h kmditoolviewaccessor.h

METASOURCES = AUTO

include ../admin/Doxyfile.am
