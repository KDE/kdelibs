kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

### Process this file with automake to produce Makefile.in
#
#AUTOMAKE_OPTIONS = no-dependencies foreign
#
#noinst_HEADERS = ltdl.h ltdl_win.h
#noinst_LTLIBRARIES = libltdlc.la
#
#libltdlc_la_SOURCES = ltdl.c
#libltdlc_la_LIBADD = $(LIBADD_DL)
#
### Because we do not have automatic dependency tracking:
#ltdl.lo: $(srcdir)/ltdl.h $(top_builddir)/config.h
#
#apidox:
#
#.PHONY: apidox
#
