kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(knotifyconfig_LIB_SRCS
knotifyconfigactionswidget.cpp
knotifyconfigelement.cpp
knotifyeventlist.cpp
knotifyconfigwidget.cpp
)

kde4_automoc(${knotifyconfig_LIB_SRCS})

set( knotifyconfig_UI knotifyconfigactionswidgetbase.ui )

kde4_add_ui_files(knotifyconfig_LIB_SRCS ${knotifyconfig_UI} )

kde4_add_library(knotifyconfig SHARED ${knotifyconfig_LIB_SRCS})

target_link_libraries(knotifyconfig  ${QT_AND_KDECORE_LIBS} kdeui kio)

set_target_properties(knotifyconfig PROPERTIES VERSION 1.0.1 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} knotifyconfig )


########### install files ###############
install_files( /include FILES knotifyconfigwidget.h )


kde4_footer()



#original Makefile.am contents follow:

#
#
#INCLUDES= -I$(srcdir) $(all_includes)
#
#lib_LTLIBRARIES = libknotifyconfig.la
#
#libknotifyconfig_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:1 -no-undefined
#libknotifyconfig_la_LIBADD = $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_DCOP) $(LIB_KFILE)
#libknotifyconfig_la_SOURCES =   knotifyconfigactionswidgetbase.ui knotifyconfigactionswidget.cpp knotifyconfigelement.cpp knotifyeventlist.cpp knotifyconfigwidget.cpp
#
#libknotifyconfig_la_METASOURCES = AUTO
#
#include_HEADERS = knotifyconfigwidget.h
