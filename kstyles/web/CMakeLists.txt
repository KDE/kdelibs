kde4_header()

project(kstyle-web)

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(webstyle_LIB_SRCS webstyle.cpp plugin.cpp )

kde4_automoc(${webstyle_LIB_SRCS})

kde4_add_library(webstyle SHARED ${webstyle_LIB_SRCS})

target_link_libraries(webstyle  ${QT_AND_KDECORE_LIBS} kdeui )

set_target_properties(webstyle PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${KDE4_LIB_INSTALL_DIR} webstyle )


########### install files ###############

install_files( ${KDE4_DATA_DIR}/kstyle/themes FILES web.themerc )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES              = $(all_includes)
#lib_LTLIBRARIES       = webstyle.la
#webstyle_la_LDFLAGS   = $(all_libraries) -module -avoid-version -no-undefined 
#webstyle_la_LIBADD    = $(LIB_KDEUI)
#webstyle_la_SOURCES   = webstyle.cpp plugin.cpp
#noinst_HEADERS        = webstyle.h 
#
#kstylewebdata_DATA = web.themerc
#kstylewebdatadir = $(kde_datadir)/kstyle/themes
#EXTRA_DIST = $(kstylewebdata_DATA)
