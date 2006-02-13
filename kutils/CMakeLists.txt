kde4_header()

project(kutils)

add_subdirectory( ksettings )
add_subdirectory( tests )

include_directories( ${CMAKE_SOURCE_DIR}/interfaces/kregexpeditor ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )

# kutils/ksettings/Makefile.am: ksettings

set(ksettings_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/kutils/ksettings/dispatcher.cpp
  ${CMAKE_SOURCE_DIR}/kutils/ksettings/dialog.cpp
  ${CMAKE_SOURCE_DIR}/kutils/ksettings/pluginpage.cpp
  ${CMAKE_SOURCE_DIR}/kutils/ksettings/componentsdialog.cpp
)

########### next target ###############

set(kutils_LIB_SRCS
kfind.cpp
kfinddialog.cpp
kreplace.cpp
kreplacedialog.cpp
kcmoduleinfo.cpp
kcmoduleloader.cpp
kcmultidialog.cpp
kmultitabbar.cpp
kplugininfo.cpp
kcmoduleproxy.cpp
kpluginselector.cpp
kcmodulecontainer.cpp
kcmoduleproxyIfaceImpl.cpp
${ksettings_STAT_SRCS}
)

kde4_automoc(${kutils_LIB_SRCS})

set( kutils_DCOP_SKEL_SRCS
kcmoduleproxyIface.h
)

kde4_add_dcop_skels(kutils_LIB_SRCS ${kutils_DCOP_SKEL_SRCS})

kde4_add_library(kutils SHARED ${kutils_LIB_SRCS})

target_link_libraries(kutils  ${QT_AND_KDECORE_LIBS} kdeui kio)

set_target_properties(kutils PROPERTIES VERSION 1.2.0 SOVERSION 1 )
install_targets(${KDE4_LIB_INSTALL_DIR} kutils )


########### install files ###############

install_files( ${KDE4_SERVICETYPES_DIR} FILES kplugininfo.desktop )
install_files( /include FILES kfind.h kfinddialog.h kreplace.h kreplacedialog.h                    kcmoduleinfo.h kcmoduleloader.h kcmultidialog.h                    kmultitabbar.h kplugininfo.h kcmoduleproxy.h                    kpluginselector.h kcmodulecontainer.h )

kde4_create_doxygen_docs( REFERENCES kdecore kdeui kio  )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = ksettings . tests
#INCLUDES = -I$(top_srcdir)/interfaces/kregexpeditor $(all_includes)
#
### TODO
### KDE_CXXFLAGS = -UQT3_SUPPORT
#
#lib_LTLIBRARIES   = libkutils.la
#libkutils_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 3:0:2
## Needs KTrader
#libkutils_la_LIBADD  = ../kio/libkio.la ksettings/libksettings.la
#
#libkutils_la_SOURCES = kfind.cpp kfinddialog.cpp kreplace.cpp \
#                       kreplacedialog.cpp kcmoduleinfo.cpp kcmoduleloader.cpp \
#                       kcmultidialog.cpp kmultitabbar.cpp kplugininfo.cpp \
#                       kcmoduleproxy.cpp kpluginselector.cpp \
#                       kcmodulecontainer.cpp kcmoduleproxyIface.skel \
#                       kcmoduleproxyIfaceImpl.cpp
#
#include_HEADERS = kfind.h kfinddialog.h kreplace.h kreplacedialog.h \
#                  kcmoduleinfo.h kcmoduleloader.h kcmultidialog.h \
#                  kmultitabbar.h kplugininfo.h kcmoduleproxy.h \
#                  kpluginselector.h kcmodulecontainer.h
#
#noinst_HEADERS = kmultitabbar_p.h kpluginselector_p.h \
#                 kcmoduleproxyIface.h kcmoduleproxyIfaceImpl.h
#
#kde_servicetypes_DATA = kplugininfo.desktop
#
#METASOURCES = AUTO
#
#DOXYGEN_REFERENCES = kdecore kdeui kio
#include ../admin/Doxyfile.am
#
## vim: et
