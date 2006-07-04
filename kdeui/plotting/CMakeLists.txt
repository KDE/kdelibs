

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
install(TARGETS kdeeduplot  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install_files(/include/libkdeedu FILES
   libkdeedu_plot_export.h
   kplotobject.h
   kplotaxis.h
   kplotwidget.h
)


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
