
project(kresources)

add_subdirectory( tests )

include_directories( ${KDE4_KIO_INCLUDES} )


########### next target ###############

set(kresources_LIB_SRCS
   resource.cpp
   factory.cpp
   managerimpl.cpp
   configwidget.cpp
   configdialog.cpp
   selectdialog.cpp
   configpage.cpp
   idmapper.cpp
   )

kde4_automoc(${kresources_LIB_SRCS})

set( kresources_DCOP_SKEL_SRCS manageriface.h )

kde4_add_dcop_skels(kresources_LIB_SRCS ${kresources_DCOP_SKEL_SRCS})

set( kresources_DCOP_STUB_SRCS manageriface.h )

kde4_add_dcop_stubs(kresources_LIB_SRCS ${kresources_DCOP_STUB_SRCS})

kde4_add_library(kresources SHARED ${kresources_LIB_SRCS})

target_link_libraries(kresources  ${KDE4_KDECORE_LIBS} kdeui kio )

set_target_properties(kresources PROPERTIES VERSION 2.0.0 SOVERSION 2 )
install_targets(${LIB_INSTALL_DIR} kresources )


########### next target ###############

set(kcm_kresources_PART_SRCS kcmkresources.cpp )

kde4_automoc(${kcm_kresources_PART_SRCS})

kde4_add_plugin(kcm_kresources ${kcm_kresources_PART_SRCS})

target_link_libraries(kcm_kresources  ${KDE4_KDECORE_LIBS} kresources )

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kcm_kresources )
install_targets(${PLUGIN_INSTALL_DIR} kcm_kresources )

########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kresources.desktop )
install_files( ${SERVICETYPES_INSTALL_DIR} FILES kresources_plugin.desktop kresources_manager.desktop )
install_files( /include/kresources FILES resource.h    configwidget.h    factory.h    manageriface.h    managerimpl.h    manager.h    selectdialog.h    configdialog.h    configpage.h idmapper.h )




#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#
#METASOURCES = AUTO
#
#lib_LTLIBRARIES = libkresources.la
#
#libkresources_la_SOURCES = resource.cpp \
#  factory.cpp \
#  manageriface.skel \
#  manageriface.stub \
#  managerimpl.cpp \
#  configwidget.cpp \
#  configdialog.cpp \
#  selectdialog.cpp \
#  configpage.cpp
#
#libkresources_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 3:0:2
#libkresources_la_LIBADD = $(LIB_KDEUI) $(LIB_KIO)
#
#kresincludedir = $(includedir)/kresources
#kresinclude_HEADERS = resource.h \
#  configwidget.h \
#  factory.h \
#  manageriface.h \
#  managerimpl.h \
#  manager.h \
#  selectdialog.h \
#  configdialog.h \
#  configpage.h
#
#kde_module_LTLIBRARIES = kcm_kresources.la
#
#kcm_kresources_la_SOURCES = kcmkresources.cpp
#kcm_kresources_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
#kcm_kresources_la_LIBADD = libkresources.la
#
#
#check_PROGRAMS = testresources
#
#testresources_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#testresources_LDADD   = libkresources.la
#testresources_SOURCES = testresources.cpp
#
#xdg_apps_DATA = kresources.desktop
#
#servicetypedir = $(kde_servicetypesdir)
#servicetype_DATA = kresources_plugin.desktop kresources_manager.desktop
#
#include ../admin/Doxyfile.am
