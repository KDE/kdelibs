kde4_header()

project(kio)

macro_optional_find_package(FAM)

add_definitions(-D_LARGEFILE64_SOURCE)

if(OPENSSL_FOUND)
   set(KSSL_HAVE_SSL 1)
   include_directories(${OPENSSL_INCLUDE_DIR})
endif(OPENSSL_FOUND)

# This applies to all subdirs; let's put kio's dependencies (and own dirs) here, once and for all
include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/bookmarks
   ${CMAKE_CURRENT_SOURCE_DIR}/kio
   ${CMAKE_CURRENT_SOURCE_DIR}/kfile
   ${CMAKE_CURRENT_SOURCE_DIR}/kssl
   ${CMAKE_CURRENT_BINARY_DIR}/kio
   ${CMAKE_CURRENT_BINARY_DIR}/kssl
   ${CMAKE_CURRENT_BINARY_DIR}/kfile
   ${CMAKE_CURRENT_BINARY_DIR}      # e.g. for observer_stub.h
   ${CMAKE_SOURCE_DIR}/kwallet/client
   ${ZLIB_INCLUDE_DIR}
)
include_directories( ${KDE4_KDEUI_INCLUDES} )

configure_file(kssl/ksslconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kssl/ksslconfig.h )
#macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/kssl/ksslconfig.h )

add_subdirectory( kssl )
add_subdirectory( kio ) 
add_subdirectory( bookmarks ) 
add_subdirectory( kfile ) 
add_subdirectory( kpasswdserver ) 
if (UNIX)
   add_subdirectory( misc ) 
endif (UNIX)
add_subdirectory( pics ) 
add_subdirectory( tests ) 
add_subdirectory( kioexec ) 

# for the kfile usage in bookmarks
include_directories( ${CMAKE_SOURCE_DIR}/kio/kfile )

# kio/kio/Makefile.am: kiocore

set(kiocore_STAT_SRCS
  kio/kdirwatch.cpp
  kio/kfileshare.cpp
  kio/ksambashare.cpp
  kio/knfsshare.cpp
  kio/ktrader.cpp
  kio/ktraderparse.cpp
  kio/ktraderparsetree.cpp
  kio/kservicetypefactory.cpp
  kio/kservicetype.cpp
  kio/kmimetype.cpp
  kio/kdedesktopmimetype.cpp
  kio/kservicegroup.cpp
  kio/kservice.cpp
  kio/kservicefactory.cpp
  kio/kuserprofile.cpp
  kio/kservicegroupfactory.cpp
  kio/kurifilter.cpp
  kio/kshellcompletion.cpp
  kio/kurlcompletion.cpp
  kio/krun.cpp
  kio/kfileitem.cpp
  kio/kdirlister.cpp
  kio/kimageio.cpp
  kio/yacc.c
  kio/lex.c
  kio/chmodjob.cpp
  kio/kscan.cpp
  kio/kar.cpp
  kio/ktar.cpp
  kio/kzip.cpp
  kio/previewjob.cpp
  kio/metainfojob.cpp
  kio/davjob.cpp
  kio/kdatatool.cpp
  kio/karchive.cpp
  kio/kfilefilter.cpp
  kio/kfilemetainfo.cpp
  kio/kdcopservicestarter.cpp
  kio/dataslave.cpp
  kio/dataprotocol.cpp
  kio/kacl.cpp
  kio/authinfo.cpp
  kio/kprotocolmanager.cpp
  kio/slave.cpp
  kio/slaveinterface.cpp
  kio/sessiondata.cpp
  kio/scheduler.cpp
  kio/connection.cpp
  kio/job.cpp
  kio/global.cpp
  kio/slaveconfig.cpp
  kio/kurlpixmapprovider.cpp
  kio/netaccess.cpp
  kio/paste.cpp
  kio/pastedialog.cpp
  kio/kmimemagic.cpp
  kio/tcpslavebase.cpp
  kio/slavebase.cpp
  kio/passdlg.cpp
  kio/forwardingslavebase.cpp
  kio/progressbase.cpp
  kio/defaultprogress.cpp
  kio/statusbarprogress.cpp
  kio/kdirnotify.cpp
  kio/kdirnotify_stub.cpp
  kio/observer.cpp
  kio/kemailsettings.cpp
  kio/kprotocolinfo.cpp
  kio/renamedlg.cpp
  kio/skipdlg.cpp
  kio/kremoteencoding.cpp
  kio/kmimetypechooser.cpp
)

if(UNIX)
    set(kiocore_STAT_SRCS ${kiocore_STAT_SRCS} ${CMAKE_SOURCE_DIR}/kio/kio/kautomount.cpp )
endif(UNIX)


# kio/bookmarks/Makefile.am: kbookmarks

set(kbookmarks_STAT_SRCS
  bookmarks/kbookmark.cc
  bookmarks/kbookmarkbar.cc
  bookmarks/kbookmarkimporter.cc
  bookmarks/kbookmarkmanager.cc
  bookmarks/kbookmarkmenu.cc
  bookmarks/kbookmarkimporter_crash.cc
  bookmarks/kbookmarkimporter_opera.cc
  bookmarks/kbookmarkimporter_ie.cc
  bookmarks/kbookmarkimporter_ns.cc
  bookmarks/kbookmarkimporter_kde1.cc
  bookmarks/kbookmarkdombuilder.cc
)

# kio/kfile/Makefile.am: kfile

set(kfile_STAT_SRCS
  kfile/kfilefiltercombo.cpp
  kfile/kfileview.cpp
  kfile/kfileiconview.cpp
  kfile/krecentdocument.cpp
  kfile/kfiledialog.cpp
  kfile/kdiroperator.cpp
  kfile/kfiledetailview.cpp
  kfile/kcombiview.cpp
  kfile/kurlrequester.cpp
  kfile/kfilepreview.cpp
  kfile/kurlcombobox.cpp
  kfile/kurlrequesterdlg.cpp
  kfile/kopenwith.cpp
  kfile/kpropertiesdialog.cpp
  kfile/kicondialog.cpp
  kfile/kdirsize.cpp
  kfile/krecentdirs.cpp
  kfile/kdiskfreesp.cpp
  kfile/kimagefilepreview.cpp
  kfile/kfilesharedlg.cpp
  kfile/kurlbar.cpp
  kfile/kmetaprops.cpp
  kfile/kpreviewprops.cpp
  kfile/kfiletreeview.cpp
  kfile/kfiletreeviewitem.cpp
  kfile/kfiletreebranch.cpp
  kfile/kdirselectdialog.cpp
  kfile/kfilebookmarkhandler.cpp
  kfile/kfilemetainfowidget.cpp
  kfile/kcustommenueditor.cpp
  kfile/knotifydialog.cpp
  kfile/kfilespeedbar.cpp
  kfile/kpreviewwidgetbase.cpp
  kfile/kfilemetapreview.cpp
  kfile/kencodingfiledialog.cpp
  kfile/kacleditwidget.cpp
)

# kio/kssl/Makefile.am: kssl

set(kssl_STAT_SRCS
  kssl/kssl.cc
  kssl/ksslcertificatehome.cc
  kssl/ksslsettings.cc
  kssl/ksslcertificate.cc
  kssl/ksslconnectioninfo.cc
  kssl/ksslcertificatefactory.cc
  kssl/ksslcertificatecache.cc
  kssl/ksslpeerinfo.cc
  kssl/ksslinfodlg.cc
  kssl/ksslcertdlg.cc
  kssl/ksslutils.cc
  kssl/kopenssl.cc
  kssl/ksslpkcs12.cc
  kssl/ksslcertchain.cc
  kssl/ksslkeygen.cc
  kssl/ksslx509v3.cc
  kssl/ksslsigners.cc
  kssl/ksslpkcs7.cc
  kssl/ksslpemcallback.cc
  kssl/ksslx509map.cc
  kssl/ksslsession.cc
  kssl/ksmimecrypto.cc
  kssl/ksslcsessioncache.cc
)


########### next target ###############

set(kio_LIB_SRCS
   ${kiocore_STAT_SRCS}
   ${kbookmarks_STAT_SRCS}
   ${kfile_STAT_SRCS}
   ${kssl_STAT_SRCS}
)

set(kio_STUB_HEADERS
   misc/uiserver.h
   kio/observer.h
)

kde4_add_dcop_stubs(kio_LIB_SRCS ${kio_STUB_HEADERS} )

set(kio_SKEL_HEADERS
   #misc/uiserver.h
   kio/observer.h
   kio/kdirnotify.h
   bookmarks/kbookmarkmanager.h
   bookmarks/kbookmarknotifier.h
)

kde4_add_dcop_skels(kio_LIB_SRCS ${kio_SKEL_HEADERS} )

kde4_add_ui_files(kio_LIB_SRCS
   kfile/kpropertiesdesktopadvbase.ui
   kfile/kpropertiesdesktopbase.ui
   kfile/kpropertiesmimetypebase.ui
   kfile/knotifywidgetbase.ui
   kssl/keygenwizard.ui
   kssl/keygenwizard2.ui
)

kde4_automoc(${kio_LIB_SRCS})

kde4_add_library(kio SHARED ${kio_LIB_SRCS})

target_link_libraries(kio  ${KDE4_KDECORE_LIBS} ${QT_QT3SUPPORT_LIBRARY} kdeui kwalletclient ${ZLIB_LIBRARY} )

if (UNIX)
   target_link_libraries(kio kdesu)
else (UNIX)
   #target_link_libraries(kio ltdlc)
endif (UNIX)

if(FAM_FOUND)
   include_directories(${FAM_INCLUDE_DIRECTORY})
   target_link_libraries(kio ${FAM_LIBRARIES})
endif(FAM_FOUND)

if(ACL_FOUND)
   target_link_libraries(kio ${ACL_LIBS})
endif(ACL_FOUND)

set_target_properties(kio PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
########### install files ###############

install_targets(${LIB_INSTALL_DIR} kio )

install_files(${KCONF_UPDATE_INSTALL_DIR} FILES useragent.pl proxytype.pl )

install_files( ${MIME_INSTALL_DIR} FILES magic )
install_files( ${SERVICETYPES_INSTALL_DIR} FILES application.desktop kurifilterplugin.desktop kscan.desktop kdatatool.desktop kfileplugin.desktop kcmodule.desktop )
install_files( ${SERVICES_INSTALL_DIR} FILES data.protocol )
install_files( ${SERVICES_INSTALL_DIR}/kded FILES kpasswdserver.desktop )
install_files( ${SERVICETYPES_INSTALL_DIR} FILES renamedlgplugin.desktop )
install_files( ${DATA_INSTALL_DIR}/kconf_update FILES kioslave.upd )

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
