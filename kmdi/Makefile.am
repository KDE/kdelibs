INCLUDES = -I$(top_srcdir)/kmdi/res $(all_includes) -I$(top_srcdir)/kutils

lib_LTLIBRARIES = libkmdi.la
libkmdi_la_LDFLAGS = $(all_libraries) -version-info 1:0
libkmdi_la_LIBADD  = $(LIB_KPARTS) ../kutils/libkutils.la

libkmdi_la_SOURCES = kmdichildarea.cpp kmdichildfrm.cpp kmdichildfrmcaption.cpp kmdichildview.cpp kmdimainfrm.cpp kmditaskbar.cpp kmdidockcontainer.cpp

include_HEADERS = kmdichildarea.h kmdichildfrm.h kmdichildfrmcaption.h kmdidefines.h \
                  kmdiiterator.h kmdilistiterator.h kmdimainfrm.h kmdinulliterator.h \
                  kmditaskbar.h kmdichildview.h

METASOURCES = AUTO
