INCLUDES = $(LIBART_CFLAGS) $(all_includes)

noinst_LTLIBRARIES = libkdesvgicons.la

libkdesvgicons_la_SOURCES = ksvgiconengine.cpp ksvgiconpainter.cpp

bin_PROGRAMS = ksvgtopng
ksvgtopng_SOURCES = ksvgtopng.cpp
ksvgtopng_LDADD = $(LIBART_LIBS) libkdesvgicons.la $(LIB_QT) $(all_libraries)

