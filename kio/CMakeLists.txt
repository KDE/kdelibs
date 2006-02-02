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

# kio/kio/Makefile.am: kiocore

set(kiocore_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/kio/kio/kdirwatch.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kfileshare.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/ksambashare.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/knfsshare.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/ktrader.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/ktraderparse.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/ktraderparsetree.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kservicetypefactory.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kservicetype.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kmimetype.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kservicegroup.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kservice.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kservicefactory.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kuserprofile.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kservicegroupfactory.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kurifilter.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kshellcompletion.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kurlcompletion.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kautomount.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/krun.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kfileitem.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kdirlister.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kimageio.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/yacc.c
  ${CMAKE_SOURCE_DIR}/kio/kio/lex.c
  ${CMAKE_SOURCE_DIR}/kio/kio/chmodjob.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kscan.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kar.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/ktar.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kzip.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/previewjob.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/metainfojob.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/davjob.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kdatatool.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/karchive.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kfilefilter.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kfilemetainfo.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kdcopservicestarter.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/dataslave.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/dataprotocol.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kacl.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/authinfo.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kprotocolmanager.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/slave.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/slaveinterface.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/sessiondata.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/scheduler.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/connection.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/job.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/global.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/slaveconfig.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kurlpixmapprovider.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/netaccess.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/paste.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/pastedialog.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kmimemagic.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/tcpslavebase.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/slavebase.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/passdlg.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/forwardingslavebase.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/progressbase.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/defaultprogress.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/statusbarprogress.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kdirnotify.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kdirnotify_stub.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/observer.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kemailsettings.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kprotocolinfo.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/renamedlg.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/skipdlg.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kremoteencoding.cpp
  ${CMAKE_SOURCE_DIR}/kio/kio/kmimetypechooser.cpp
)

# kio/bookmarks/Makefile.am: kbookmarks

set(kbookmarks_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/kio/bookmarks/kbookmark.cc
  ${CMAKE_SOURCE_DIR}/kio/bookmarks/kbookmarkbar.cc
  ${CMAKE_SOURCE_DIR}/kio/bookmarks/kbookmarkimporter.cc
  ${CMAKE_SOURCE_DIR}/kio/bookmarks/kbookmarkmanager.cc
  ${CMAKE_SOURCE_DIR}/kio/bookmarks/kbookmarkmenu.cc
  ${CMAKE_SOURCE_DIR}/kio/bookmarks/kbookmarkimporter_crash.cc
  ${CMAKE_SOURCE_DIR}/kio/bookmarks/kbookmarkimporter_opera.cc
  ${CMAKE_SOURCE_DIR}/kio/bookmarks/kbookmarkimporter_ie.cc
  ${CMAKE_SOURCE_DIR}/kio/bookmarks/kbookmarkimporter_ns.cc
  ${CMAKE_SOURCE_DIR}/kio/bookmarks/kbookmarkimporter_kde1.cc
  ${CMAKE_SOURCE_DIR}/kio/bookmarks/kbookmarkdombuilder.cc
)

# kio/kfile/Makefile.am: kfile

set(kfile_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/kio/kfile/kfilefiltercombo.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kfileview.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kfileiconview.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/krecentdocument.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kfiledialog.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kdiroperator.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kfiledetailview.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kcombiview.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kurlrequester.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kfilepreview.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kurlcombobox.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kurlrequesterdlg.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kopenwith.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kpropertiesdialog.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kicondialog.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kdirsize.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/krecentdirs.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kdiskfreesp.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kimagefilepreview.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kfilesharedlg.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kurlbar.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kmetaprops.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kpreviewprops.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kfiletreeview.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kfiletreeviewitem.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kfiletreebranch.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kdirselectdialog.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kfilebookmarkhandler.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kfilemetainfowidget.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kcustommenueditor.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/knotifydialog.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kfilespeedbar.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kpreviewwidgetbase.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kfilemetapreview.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kencodingfiledialog.cpp
  ${CMAKE_SOURCE_DIR}/kio/kfile/kacleditwidget.cpp
)

# kio/kssl/Makefile.am: kssl

set(kssl_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/kio/kssl/kssl.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslcertificatehome.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslsettings.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslcertificate.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslconnectioninfo.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslcertificatefactory.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslcertificatecache.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslpeerinfo.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslinfodlg.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslcertdlg.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslutils.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/kopenssl.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslpkcs12.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslcertchain.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslkeygen.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslx509v3.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslsigners.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslpkcs7.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslpemcallback.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslx509map.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslsession.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksmimecrypto.cc
  ${CMAKE_SOURCE_DIR}/kio/kssl/ksslcsessioncache.cc
)


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
install_targets(${KDE4_LIB_INSTALL_DIR} kio )


########### install files ###############

install_files( ${KDE4_MIME_DIR} FILES magic )
install_files( ${KDE4_SERVICETYPES_DIR} FILES application.desktop kurifilterplugin.desktop kscan.desktop kdatatool.desktop kfileplugin.desktop kcmodule.desktop )
install_files( ${KDE4_SERVICES_DIR} FILES data.protocol )
install_files( ${KDE4_SERVICES_DIR}/kded FILES kpasswdserver.desktop )
install_files( ${KDE4_SERVICETYPES_DIR} FILES renamedlgplugin.desktop )
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
