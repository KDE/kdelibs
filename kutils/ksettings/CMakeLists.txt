kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES=-I$(srcdir)/.. $(all_includes)
#noinst_LTLIBRARIES = libksettings.la
#
#libksettings_la_SOURCES = dispatcher.cpp dialog.cpp pluginpage.cpp componentsdialog.cpp
#
#ksettingsincludedir = $(includedir)/ksettings
#ksettingsinclude_HEADERS = dispatcher.h dialog.h pluginpage.h componentsdialog.h
#
#METASOURCES = AUTO
