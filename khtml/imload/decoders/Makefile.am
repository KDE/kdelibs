noinst_LTLIBRARIES = libdecoders.la
libdecoders_la_SOURCES = jpegloader.cpp pngloader.cpp gifloader.cpp

libdecoders_la_METASOURCES = AUTO
libdecoders_la_LIBADD = -ljpeg $(GIF_LIBRARY)
noinst_HEADERS = jpegloader.h pngloader.h gifloader.h

INCLUDES = -I$(top_srcdir)/khtml/imload $(all_includes)

