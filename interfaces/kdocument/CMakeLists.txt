kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/interfaces ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kdocument_LIB_SRCS
kdocument.cpp
)

kde4_automoc(${kdocument_LIB_SRCS})

kde4_add_library(kdocument SHARED ${kdocument_LIB_SRCS})

target_link_libraries(kdocument  ${QT_AND_KDECORE_LIBS} kparts )

set_target_properties(kdocument PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(/lib kdocument )


########### install files ###############
install_files( /include/kdocument FILES factory.h document.h view.h )


kde4_create_doxygen_docs( REFERENCES kparts  )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/interfaces $(all_includes)
#
#lib_LTLIBRARIES = libkdocument.la
#
#libkdocument_la_SOURCES = kdocument.cpp
#
#libkdocument_la_LDFLAGS = -no-undefined $(all_libraries) $(KDE_RPATH)
#libkdocument_la_LIBADD = $(LIB_KPARTS)
#
#kdocumentinclude_HEADERS = factory.h document.h view.h
#
#kdocumentincludedir = $(includedir)/kdocument
#
#METASOURCES = AUTO
#
##servicetypedir = $(kde_servicetypesdir)
##servicetype_DATA = kdocument.desktop kdocumentplugin.desktop
#
##kcm_kdocument_DATA = kcm_kdocument.desktop
##kcm_kdocumentdir = $(kde_datadir)/kcm_componentchooser
#
#DOXYGEN_REFERENCES = kparts
#include ../../admin/Doxyfile.am
#
