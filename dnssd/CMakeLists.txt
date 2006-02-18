kde4_header()

project(dnssd)

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kdnssd_LIB_SRCS
   remoteservice.cpp
   responder.cpp
   servicebase.cpp
   publicservice.cpp
   query.cpp
   domainbrowser.cpp
   servicebrowser.cpp
   )

kde4_automoc(${kdnssd_LIB_SRCS})

set( kdnssd_KCFG_SRCS settings.kcfgc )

kde4_add_kcfg_files(kdnssd_LIB_SRCS ${kdnssd_KCFG_SRCS})

kde4_add_library(kdnssd SHARED ${kdnssd_LIB_SRCS})

target_link_libraries(kdnssd  ${QT_AND_KDECORE_LIBS} kdecore )

set_target_properties(kdnssd PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} kdnssd )


########### install files ###############
install_files( /include/dnssd FILES domainbrowser.h query.h remoteservice.h   publicservice.h servicebase.h servicebrowser.h settings.h )


kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

## set the include path for X, qt and KDE
#INCLUDES = -I$(top_srcdir) $(all_includes)
#
## these are the headers for your project
#noinst_HEADERS = sdevent.h
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#lib_LTLIBRARIES =  libkdnssd.la
#
#libkdnssd_la_SOURCES = remoteservice.cpp responder.cpp servicebase.cpp \
#				settings.kcfgc publicservice.cpp query.cpp domainbrowser.cpp servicebrowser.cpp
#dnssdincludedir = $(includedir)/dnssd
#dnssdinclude_HEADERS = domainbrowser.h query.h remoteservice.h \
#	publicservice.h servicebase.h servicebrowser.h settings.h
#libkdnssd_la_LIBADD = ../kdecore/libkdecore.la $(LIB_DNSSD)
#libkdnssd_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -version-info 1:0
#
##kde_kcfg_DATA = kcm_kdnssd.kcfg
#
#include ../admin/Doxyfile.am
