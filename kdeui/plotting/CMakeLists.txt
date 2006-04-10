kde4_header()

include_directories(
   ${KDE4_INCLUDE_DIR}
   ${QT_INCLUDES}
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
)


########### next target ###############

set(kdeeduplot_LIB_SRCS
   kplotobject.cpp
   kplotaxis.cpp
   kplotwidget.cpp
)

kde4_automoc(${kdeeduplot_LIB_SRCS})

kde4_add_library(kdeeduplot SHARED ${kdeeduplot_LIB_SRCS})

target_link_libraries(kdeeduplot  ${KDE4_KDEUI_LIBS} )

set_target_properties(kdeeduplot PROPERTIES VERSION 1.2.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} kdeeduplot )


########### install files ###############

install_files(/include/libkdeedu FILES
   kplotobject.h
   kplotaxis.h
   kplotwidget.h
)

kde4_footer()

#original Makefile.am contents follow:

#INCLUDES= $(all_includes)
#SUBDIRS = . 
#KDE_CXXFLAGS = -UQT3_SUPPORT
#
#lib_LTLIBRARIES = libkdeeduplot.la
#
#libkdeeduplotincludedir = $(includedir)/libkdeedu
#libkdeeduplotinclude_HEADERS = kplotobject.h kplotaxis.h kplotwidget.h
#
#libkdeeduplot_la_SOURCES = kplotobject.cpp kplotaxis.cpp kplotwidget.cpp
#
#libkdeeduplot_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 3:0:2
#libkdeeduplot_la_LIBADD = $(LIB_KDECORE) $(LIB_QT)
#
#METASOURCES = AUTO
#
