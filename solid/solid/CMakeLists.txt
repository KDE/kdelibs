kde4_header()

add_subdirectory( ifaces )

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_SOURCE_DIR} )


########### next target ###############

set(kdehw_LIB_SRCS 
   device.cpp 
   devicemanager.cpp 
   capability.cpp 
   processor.cpp 
   block.cpp 
   storage.cpp 
   cdrom.cpp 
   volume.cpp 
   opticaldisc.cpp 
   camera.cpp 
   portablemediaplayer.cpp 
   display.cpp 
   predicate.cpp 
   predicateparse.cpp 
   predicate_lexer.c 
   predicate_parser.c )

kde4_automoc(${kdehw_LIB_SRCS})

kde4_add_library(kdehw SHARED ${kdehw_LIB_SRCS})

target_link_libraries(kdehw  ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} kdehwifaces )

set_target_properties(kdehw PROPERTIES VERSION 4.2.0 SOVERSION 4 )
install_targets(${LIB_INSTALL_DIR} kdehw )


########### install files ###############

install_files( ${SERVICETYPES_INSTALL_DIR} FILES  kdehwdevicemanager.desktop )
install_files( /include/kdehw FILES  device.h devicemanager.h capability.h processor.h block.h storage.h cdrom.h volume.h opticaldisc.h camera.h portablemediaplayer.h display.h predicate.h )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#
#SUBDIRS = . ifaces
#
#lib_LTLIBRARIES  = libkdehw.la 
#libkdehw_la_LIBADD = $(LIB_QTCORE) $(LIB_QT) $(LIB_KDECORE) $(LIB_KIO) ifaces/libkdehwifaces.la
#libkdehw_la_LDFLAGS = $(KDE_RPATH) -version-info 6:0:2 -no-undefined $(all_libraries)
#
#include_HEADERS = device.h devicemanager.h capability.h processor.h block.h storage.h cdrom.h volume.h opticaldisc.h camera.h portablemediaplayer.h display.h predicate.h
#
#METASOURCES = AUTO
#
#libkdehw_la_SOURCES = device.cpp devicemanager.cpp capability.cpp processor.cpp block.cpp storage.cpp cdrom.cpp volume.cpp opticaldisc.cpp camera.cpp portablemediaplayer.cpp display.cpp predicate.cpp predicateparse.cpp predicate_lexer.c predicate_parser.c
#
#
#kde_servicetypes_DATA = kdehwdevicemanager.desktop
#
#
#parserfiles = predicate_parser.y predicate_lexer.l
#
#EXTRA_DIST = $(parserfiles)
#
#parser: $(parserfiles)
#	cd $(srcdir) ;\
#	flex -opredicate_lexer.c predicate_lexer.l ;\
#	bison -d predicate_parser.y &&\
#	mv predicate_parser.tab.c predicate_parser.c &&\
#	mv predicate_parser.tab.h predicate_parser.h
#
#.PHONY: parser
#
#
