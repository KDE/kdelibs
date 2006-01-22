kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kded_KDEINIT_SRCS
kded.cpp
)

kde4_automoc(${kded_KDEINIT_SRCS})

kde4_add_klm( kded ${kded_KDEINIT_SRCS})

target_link_libraries(kdeinit_kded  ${QT_AND_KDECORE_LIBS} kio )

install_targets(/lib kdeinit_kded )

target_link_libraries( kded kdeinit_kded )
install_targets(/bin kded )

########### next target ###############

set(kbuildsycoca_KDEINIT_SRCS
kbuildsycoca.cpp
kbuildservicetypefactory.cpp
kbuildservicefactory.cpp
kbuildservicegroupfactory.cpp
#kbuildimageiofactory.cpp
kbuildprotocolinfofactory.cpp
kctimefactory.cpp
vfolder_menu.cpp
)

kde4_automoc(${kbuildsycoca_KDEINIT_SRCS})

kde4_add_klm( kbuildsycoca ${kbuildsycoca_KDEINIT_SRCS})

target_link_libraries(kdeinit_kbuildsycoca  ${QT_AND_KDECORE_LIBS} kio )

install_targets(/lib kdeinit_kbuildsycoca )

target_link_libraries( kbuildsycoca kdeinit_kbuildsycoca )
install_targets(/bin kbuildsycoca )

########### next target ###############

set(kdontchangethehostname_SRCS
khostname.cpp
)

kde4_automoc(${kdontchangethehostname_SRCS})

kde4_add_executable(kdontchangethehostname ${kdontchangethehostname_SRCS})

target_link_libraries(kdontchangethehostname  ${QT_AND_KDECORE_LIBS} )

install_targets(/bin kdontchangethehostname )


########### next target ###############

set(kde-menu_SRCS
kde-menu.cpp
)

kde4_automoc(${kde-menu_SRCS})

kde4_add_executable(kde-menu ${kde-menu_SRCS})

target_link_libraries(kde-menu  ${QT_AND_KDECORE_LIBS} kio)

install_targets(/bin kde-menu )


########### install files ###############

install_files(  FILES applications.menu )
install_files( ${KDE4_SERVICESTYPES_DIR} FILES kdedmodule.desktop )
install_files( ${KDE4_DATA_DIR}/kconf_update FILES kded.upd )

kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

##    This file is part of the KDE libraries
##    Copyright (C) 1997-2006 David Faure <faure@kde.org>
##    Copyright (C) 2000 Waldo Bastian <bastian@kde.org>
#
##    This library is free software; you can redistribute it and/or
##    modify it under the terms of the GNU Library General Public
##    License as published by the Free Software Foundation; either
##    version 2 of the License, or (at your option) any later version.
#
##    This library is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
##    Library General Public License for more details.
#
##    You should have received a copy of the GNU Library General Public License
##    along with this library; see the file COPYING.LIB.  If not, write to
##    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
##    Boston, MA 02110-1301, USA.
#
#INCLUDES= -I$(srcdir)/.. -I$(top_srcdir) $(all_includes)
#KDE_CXXFLAGS = -UQT3_SUPPORT
#
#kdeinit_LTLIBRARIES = kded.la kbuildsycoca.la
#
#kded_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
#kded_la_LIBADD = $(LIB_KIO)
#kded_la_SOURCES = kded.cpp
#
#kbuildsycoca_la_LDFLAGS = $(all_libraries) -module -avoid-version
#kbuildsycoca_la_LIBADD = $(LIB_KIO)
#kbuildsycoca_la_SOURCES = kbuildsycoca.cpp kbuildservicetypefactory.cpp \
#				kbuildservicefactory.cpp \
#				kbuildservicegroupfactory.cpp \
#				kbuildimageiofactory.cpp \
#				kbuildprotocolinfofactory.cpp \
#				kctimefactory.cpp \
#				vfolder_menu.cpp
#
#bin_PROGRAMS = kdontchangethehostname kde-menu
#
#kdontchangethehostname_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#kdontchangethehostname_LDADD = $(LIB_KDECORE) 
#kdontchangethehostname_SOURCES = khostname.cpp
#
#kde_menu_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#kde_menu_LDADD = $(LIB_KIO) 
#kde_menu_SOURCES = kde-menu.cpp
#
#METASOURCES = AUTO
#
#noinst_HEADERS = kbuildsycoca.h kbuildservicetypefactory.h \
#		kbuildservicefactory.h kbuildservicegroupfactory.h \
#		kbuildimageiofactory.h kresourcelist.h \
#		kbuildprotocolinfofactory.h
#
#servicetype_DATA = kdedmodule.desktop
#servicetypedir = $(kde_servicetypesdir)
#
#xdg_menu_DATA = applications.menu
#
#update_DATA = kded.upd
#updatedir = $(kde_datadir)/kconf_update
#
#include $(top_srcdir)/admin/Doxyfile.am
#
