kde4_header()


if(HAVE_SSL)
   set(KSSL_HAVE_SSL 1)
   add_definitions(-DKSSL_HAVE_SSL )
endif(HAVE_SSL)

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
${CMAKE_CURRENT_SOURCE_DIR}/bookmarks
${CMAKE_SOURCE_DIR}/kwallet/client
${CMAKE_SOURCE_DIR}/kdecore/network
${CMAKE_BINARY_DIR}/kio/kssl
)



add_subdirectory( kssl )
add_subdirectory( kio ) 
add_subdirectory( bookmarks ) 
add_subdirectory( kfile ) 
add_subdirectory( kpasswdserver ) 
add_subdirectory( misc ) 
add_subdirectory( pics ) 
add_subdirectory( tests ) 
add_subdirectory( kioexec ) 
add_subdirectory( httpfilter ) 

########### next target ###############

set(kio_LIB_SRCS
${kiocore_STAT_SRCS}
${kbookmarks_STAT_SRCS}
${kfile_STAT_SRCS}
${kssl_STAT_SRCS}
)

set(kio_STUB_HEADERS
${CMAKE_CURRENT_SOURCE_DIR}/misc/uiserver.h
${CMAKE_CURRENT_SOURCE_DIR}/kio/observer.h
)

kde4_add_dcop_stubs(kio_LIB_SRCS ${kio_STUB_HEADERS} )

set(kio_SKEL_HEADERS
#${CMAKE_CURRENT_SOURCE_DIR}/misc/uiserver.h
${CMAKE_CURRENT_SOURCE_DIR}/kio/observer.h
${CMAKE_CURRENT_SOURCE_DIR}/kio/kdirnotify.h
${CMAKE_CURRENT_SOURCE_DIR}/bookmarks/kbookmarkmanager.h
${CMAKE_CURRENT_SOURCE_DIR}/bookmarks/kbookmarknotifier.h
)

kde4_add_dcop_skels(kio_LIB_SRCS ${kio_SKEL_HEADERS} )

kde4_add_ui3_files(kio_LIB_SRCS
${CMAKE_CURRENT_SOURCE_DIR}/kfile/kpropertiesdesktopbase.ui3
${CMAKE_CURRENT_SOURCE_DIR}/kfile/kpropertiesmimetypebase.ui3
${CMAKE_CURRENT_SOURCE_DIR}/kfile/knotifywidgetbase.ui3
${CMAKE_CURRENT_SOURCE_DIR}/kssl/keygenwizard.ui3
${CMAKE_CURRENT_SOURCE_DIR}/kssl/keygenwizard2.ui3
)

kde4_add_ui_files(kio_LIB_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/kfile/kpropertiesdesktopadvbase.ui)

kde4_automoc(${kio_LIB_SRCS})

kde4_add_library(kio SHARED ${kio_LIB_SRCS})

target_link_libraries(kio  ${QT_AND_KDECORE_LIBS} ${QT_QT3SUPPORT_LIBRARY} kdeui kdesu kwalletclient ${ZLIB_LIBRARY})

if(FAM_FOUND)
   include_directories(${FAM_INCLUDE_DIRECTORY})
   target_link_libraries(kio ${FAM_LIBRARY})
endif(FAM_FOUND)
   

set_target_properties(kio PROPERTIES VERSION 4.2.0 SOVERSION 4 )
install_targets(/lib kio )


########### install files ###############

install_files( ${KDE4_MIME_DIR} FILES magic )
install_files( ${KDE4_SERVICESTYPES_DIR} FILES application.desktop kurifilterplugin.desktop                          kscan.desktop kdatatool.desktop                          kfileplugin.desktop kcmodule.desktop )
install_files( ${KDE4_SERVICES_DIR} FILES data.protocol )
install_files( ${KDE4_SERVICES_DIR}/kded FILES kpasswdserver.desktop )
install_files( ${KDE4_SERVICESTYPES_DIR} FILES renamedlgplugin.desktop )
install_files( ${KDE4_DATA_DIR}/kconf_update FILES kioslave.upd )

kde4_create_doxygen_docs( REFERENCES kdecore kdefx kjs dcop kdeui kio/kio kio/kfile EXCLUDE kssl/kssl  )

kde4_footer()



#original Makefile.am contents follow:

##    This file is part of the KDE libraries
##    Copyright (C) 1997 Torben Weis (weis@kde.org)
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
#AM_CPPFLAGS = -D_LARGEFILE64_SOURCE
#
#INCLUDES=  -I$(srcdir)/.. $(all_includes) $(SSL_INCLUDES)
#
#SUBDIRS = kssl kio bookmarks kfile . kpasswdserver misc pics tests kioexec httpfilter
#
#lib_LTLIBRARIES = libkio.la
#
#libkio_la_SOURCES = dummy.cpp
#
#libkio_la_LDFLAGS = -version-info 6:0:2 -no-undefined $(all_libraries) \
#	$(KDE_MT_LDFLAGS)
#libkio_la_LIBADD = kssl/libkssl.la kio/libkiocore.la \
#	bookmarks/libkbookmarks.la kfile/libkfile.la \
#	../kdeui/libkdeui.la ../kdesu/libkdesu.la \
#	../kwallet/client/libkwalletclient.la \
#	$(LIBZ) $(LIBFAM) $(LIBVOLMGT) $(ACL_LIBS)
#
#kde_mime_DATA = magic
#kde_servicetypes_DATA = application.desktop kurifilterplugin.desktop \
#                        kscan.desktop kdatatool.desktop \
#                        kfileplugin.desktop kcmodule.desktop
#
#EXTRA_DIST = $(kde_mime_DATA)
#
#update_DATA = kioslave.upd
#update_SCRIPTS = useragent.pl proxytype.pl
#updatedir = $(kde_datadir)/kconf_update
#
#servicetype_DATA = renamedlgplugin.desktop
#servicetypedir = $(kde_servicetypesdir)
#
#kded_DATA = kpasswdserver.desktop
#kdeddir = $(kde_servicesdir)/kded
#
#dataprotocol_DATA = data.protocol
#dataprotocoldir = $(kde_servicesdir)
#
#dummy.cpp:
#	echo >dummy.cpp
#
#messages:
#	$(EXTRACTRC) `find . ../kioslave -name "*.rc" -o -name "*.ui"` > rc.cpp
#	$(XGETTEXT) `find . ../kioslave -name "*.cpp" -o -name "*.cc" -o -name "*.h"` -o $(podir)/kio.pot
#	rm -f rc.cpp
#
#DOXYGEN_REFERENCES = kdecore kdefx kjs dcop kdeui kio/kio kio/kfile
#DOXYGEN_EXCLUDE = kssl/kssl
#include ../admin/Doxyfile.am
