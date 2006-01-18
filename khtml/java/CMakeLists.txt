kde4_header()

add_subdirectory( tests ) 
add_subdirectory( dummy ) 

include_directories( ${CMAKE_SOURCE_DIR}/khtml ${CMAKE_SOURCE_DIR}/kio/kssl ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### next target ###############

set(kjavaappletviewer_PART_SRCS
kjavaapplet.cpp
kjavaappletcontext.cpp
kjavaappletserver.cpp
kjavaappletwidget.cpp
kjavaprocess.cpp
kjavadownloader.cpp
kjavaappletviewer.cpp
)

kde4_automoc(${kjavaappletviewer_PART_SRCS})

kde4_add_plugin(kjavaappletviewer ${kjavaappletviewer_PART_SRCS})

target_link_libraries(kjavaappletviewer  ${QT_AND_KDECORE_LIBS} kparts )

install_targets(/lib/kde4 kjavaappletviewer )


########### install files ###############

install_files( ${KDE4_DATA_DIR}/kjava/ FILES kjava.jar kjava.policy pluginsinfo )
install_files( ${KDE4_SERVICES_DIR} FILES kjavaappletviewer.desktop )
install_files( ${KDE4_DATA_DIR}/kjava/icons FILES  )

kde4_footer()



#original Makefile.am contents follow:

#KDE_CXXFLAGS = $(WOVERLOADED_VIRTUAL)
#
#noinst_LTLIBRARIES = libkjava.la
#libkjava_la_SOURCES = kjavaapplet.cpp kjavaappletcontext.cpp \
#	kjavaappletserver.cpp kjavaappletwidget.cpp kjavaprocess.cpp \
#	kjavadownloader.cpp 
#
#noinst_HEADERS = kjavaappletwidget.h kqeventutil.h kxeventutil.h \
#                 kjavaapplet.h kjavaappletcontext.h \
#                 kjavaappletserver.h kjavaprocess.h kjavaappletviewer.h
#
#METASOURCES = AUTO
#
#libkjava_la_LDFLAGS = $(KDE_MT_LDFLAGS) -no-undefined
#libkjava_la_LIBADD = $(LIB_KPARTS)
#
#INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/khtml \
#	-I$(top_srcdir)/kio/kssl -I$(top_builddir)/kio/kssl \
#	$(all_includes) 
#
#kjavadata_DATA = kjava.jar kjava.policy pluginsinfo
#kjavadatadir = $(kde_datadir)/kjava/
#
#kjavaiconsdir = $(kde_datadir)/kjava/icons
#kjavaicons_ICON = AUTO
#
#SUBDIRS = . tests dummy
#
#kde_module_LTLIBRARIES= kjavaappletviewer.la
#kjavaappletviewer_la_SOURCES= kjavaapplet.cpp kjavaappletcontext.cpp \
#        kjavaappletserver.cpp kjavaappletwidget.cpp kjavaprocess.cpp \
#        kjavadownloader.cpp kjavaappletviewer.cpp
#
#kjavaappletviewer_la_LDFLAGS= $(all_libraries) -module $(KDE_PLUGIN)
#kjavaappletviewer_la_LIBADD= $(LIB_KPARTS)
#
#kde_services_DATA= kjavaappletviewer.desktop
