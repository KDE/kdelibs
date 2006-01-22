kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kio/ ${CMAKE_SOURCE_DIR}/libltdl ${CMAKE_SOURCE_DIR}/kfile ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kparts_LIB_SRCS
part.cpp
plugin.cpp
partmanager.cpp
mainwindow.cpp
event.cpp
browserextension.cpp
factory.cpp
historyprovider.cpp
browserinterface.cpp
browserrun.cpp
statusbarextension.cpp
)

kde4_automoc(${kparts_LIB_SRCS})

kde4_add_library(kparts SHARED ${kparts_LIB_SRCS})

target_link_libraries(kparts  ${QT_AND_KDECORE_LIBS} kdeui kio)

set_target_properties(kparts PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(/lib kparts )


########### install files ###############

install_files( ${KDE4_SERVICESTYPES_DIR} FILES kpart.desktop krop.desktop krwp.desktop browserview.desktop  )

kde4_create_doxygen_docs( REFERENCES kdecore kdeui kdefx kio  )

kde4_footer()



#original Makefile.am contents follow:

######## General stuff
#
#AM_LDFLAGS = $(LDFLAGS_AS_NEEDED)
#
#INCLUDES= -I$(srcdir)/../ -I$(top_srcdir)/kio/ -I$(top_srcdir)/libltdl \
#	-I$(top_srcdir)/kfile $(all_includes)
#libkparts_la_LDFLAGS = $(KDE_MT_LDFLAGS) -version-info 3:0:1 -no-undefined
#libkparts_la_LIBADD  = $(LIB_KIO)
#
#check-local:
#	(cd tests && $(MAKE) check )
#
######## Files
#
#lib_LTLIBRARIES = libkparts.la
#
#libkparts_la_SOURCES = part.cpp plugin.cpp partmanager.cpp mainwindow.cpp \
#		       event.cpp browserextension.cpp factory.cpp  historyprovider.cpp \
#		       browserinterface.cpp browserrun.cpp statusbarextension.cpp
#
#kpartsincludedir = $(includedir)/kparts
#kpartsinclude_HEADERS = part.h plugin.h partmanager.h mainwindow.h event.h \
#	browserextension.h factory.h historyprovider.h browserinterface.h genericfactory.h \
#	componentfactory.h browserrun.h statusbarextension.h
#
#libkparts_la_METASOURCES = AUTO
#
#servicetype_DATA = kpart.desktop krop.desktop krwp.desktop browserview.desktop 
#servicetypedir = $(kde_servicetypesdir)
#
#DOXYGEN_REFERENCES = kdecore kdeui kdefx kio
#include ../admin/Doxyfile.am
