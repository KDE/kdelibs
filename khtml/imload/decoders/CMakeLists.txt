kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/khtml/imload ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#noinst_LTLIBRARIES = libdecoders.la
#libdecoders_la_SOURCES = jpegloader.cpp pngloader.cpp gifloader.cpp
#
#libdecoders_la_METASOURCES = AUTO
#libdecoders_la_LIBADD = -ljpeg $(GIF_LIBRARY)
#noinst_HEADERS = jpegloader.h pngloader.h gifloader.h
#
#INCLUDES = -I$(top_srcdir)/khtml/imload $(all_includes)
#
