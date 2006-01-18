kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES= $(all_includes)
#
#noinst_LTLIBRARIES = libkab.la
#KABVERSION=	4.0.0
#KABPATCH=	0
#
#libkab_la_SOURCES	= \
#	kabapi.cc \
#	addressbook.cc \
#	qconfigDB.cc 
#
#libkab_la_LDFLAGS = $(KDE_MT_LDFLAGS) -no-undefined
#libkab_la_LIBADD = ../../kdeui/libkdeui.la
#
#libkab_la_METASOURCES = AUTO
