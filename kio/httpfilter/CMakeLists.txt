kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS = $(all_includes)
#METASOURCES = AUTO
#noinst_LTLIBRARIES = libhttpfilter.la
#libhttpfilter_la_SOURCES = httpfilter.cc
#libhttpfilter_la_LIBADD = $(LIBZ)
#libhttpfilter_la_LDFLAGS = $(all_libraries)
#
#include $(top_srcdir)/admin/Doxyfile.am
