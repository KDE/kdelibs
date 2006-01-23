kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  ${CMAKE_SOURCE_DIR}/kutils )

add_subdirectory( ui )
add_subdirectory( tests )
add_subdirectory( plugins )
add_subdirectory( kcmspellchecking )

########### next target ###############


########### install files ###############

install_files( ${KDE4_SERVICESTYPES_DIR} FILES kspellclient.desktop )
install_files( /include/kspell2 FILES settings.h filter.h   dictionary.h broker.h   backgroundchecker.h defaultdictionary.h  )

kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = . ui tests plugins kcmspellchecking
#
#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libkspell2base.la
#
#AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -I$(top_srcdir)/kutils $(all_includes)
#
#kspell2baseincludedir = $(includedir)/kspell2
#
#kspell2baseinclude_HEADERS = settings.h filter.h \
#	dictionary.h broker.h \
#	backgroundchecker.h defaultdictionary.h
#
#libkspell2base_la_SOURCES = settings.cpp broker.cpp \
#	client.cpp filter.cpp backgroundchecker.cpp \
#	backgroundthread.cpp backgroundengine.cpp \
#	defaultdictionary.cpp
#
#libkspell2base_la_LDFLAGS = -no-undefined $(all_libraries)
#libkspell2base_la_LIBADD  = $(LIB_KPARTS) ../kutils/libkutils.la
#
#servicetype_DATA = kspellclient.desktop
#servicetypedir = $(kde_servicetypesdir)
#
#include $(top_srcdir)/admin/Doxyfile.am
