INCLUDES = $(LIBART_CFLAGS) $(all_includes)

noinst_LTLIBRARIES = libkdesvgicons.la

libkdesvgicons_la_SOURCES = ksvgiconengine.cpp ksvgiconpainter.cpp

EXTRA_PROGRAMS = ksvgtopng
ksvgtopng_SOURCES = ksvgtopng.cpp
ksvgtopng_LDADD =  $(all_libraries) $(LIB_QT) $(LIBART_LIBS) $(LIB_KDECORE) libkdesvgicons.la
