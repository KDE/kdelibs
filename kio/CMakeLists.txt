
project(kio)
macro_optional_find_package(FAM)

add_definitions(-D_LARGEFILE64_SOURCE )

add_definitions(-DQT3_SUPPORT_WARNINGS -DQT3_SUPPORT)

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
   ${CMAKE_SOURCE_DIR}/kde3support/kdeui/ #for k3wizard
   ${KDE4_KDEUI_INCLUDES}
   ${ZLIB_INCLUDE_DIR}
)

macro_bool_to_01(FAM_FOUND HAVE_FAM)
check_include_files(sys/inotify.h HAVE_SYS_INOTIFY_H)
configure_file(kio/config-kdirwatch.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kio/config-kdirwatch.h )

configure_file(kssl/ksslconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kssl/ksslconfig.h )
#macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/kssl/ksslconfig.h )

add_subdirectory( kssl )
add_subdirectory( bookmarks )
add_subdirectory( kpasswdserver )
add_subdirectory( misc )
add_subdirectory( tests )
add_subdirectory( kfile/tests )
add_subdirectory( kioexec )

# for the kfile usage in bookmarks
include_directories( ${CMAKE_SOURCE_DIR}/kio/kfile )

# kio/kio/Makefile.am: kiocore

set(kiocore_STAT_SRCS
  kio/kdirwatch.cpp
  kio/kfileshare.cpp
  kio/ksambashare.cpp
  kio/knfsshare.cpp
  kio/kmimetypefactory.cpp
  kio/kmimetypetrader.cpp
  kio/kmimetype.cpp
  kio/kdedesktopmimetype.cpp
  kio/kservicegroup.cpp
  kio/kservicegroupfactory.cpp
  kio/kurifilter.cpp
  kio/kshellcompletion.cpp
  kio/kurlcompletion.cpp
  kio/krun.cpp
  kio/kfileitem.cpp
  kio/kfileitemdelegate.cpp
  kio/kdirlister.cpp
  kio/kimageio.cpp
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
  kio/kdbusservicestarter.cpp
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
  kio/jobuidelegate.cpp
  kio/filejob.cpp
  kio/copyjob.cpp
  kio/deletejob.cpp
  kio/global.cpp
  kio/slaveconfig.cpp
  kio/kurlpixmapprovider.cpp
  kio/netaccess.cpp
  kio/paste.cpp
  kio/pastedialog.cpp
  kio/kmimemagic.cpp
  kio/tcpslavebase.cpp
  kio/slavebase.cpp
  kio/passworddialog.cpp
  kio/forwardingslavebase.cpp
  kio/progressbase.cpp
  kio/defaultprogress.cpp
  kio/statusbarprogress.cpp
  kio/kdirnotify.cpp
  kio/observer.cpp
  kio/kemailsettings.cpp
  kio/renamedialog.cpp
  kio/renamedialogplugin.cpp
  kio/skipdialog.cpp
  kio/kremoteencoding.cpp
  kio/kmimetypechooser.cpp
  kio/kbuildsycocaprogressdialog.cpp
  kio/safesite.cpp
  kio/kdirmodel.cpp
  kio/kmimetyperesolver.cpp
  kio/directorysizejob.cpp
)


qt4_add_dbus_interface( kiocore_STAT_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/kio/org.kde.KIO.UIServer.xml kio/uiserveriface )

if(UNIX)
   set(kiocore_STAT_SRCS ${kiocore_STAT_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/kio/kautomount.cpp)
endif(UNIX)


# kio/bookmarks/Makefile.am: kbookmarks

set(kbookmarks_STAT_SRCS
  bookmarks/kbookmark.cc
  bookmarks/kbookmarkbar.cc
  bookmarks/kbookmarkimporter.cc
  bookmarks/kbookmarkmanager.cc
  bookmarks/kbookmarkmanageradaptor.cc
  bookmarks/kbookmarknotifier.cc
  bookmarks/kbookmarkmenu.cc
  bookmarks/konqbookmarkmenu.cc
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
  kfile/kurlrequesterdialog.cpp
  kfile/kopenwithdialog.cpp
  kfile/kpropertiesdialog.cpp
  kfile/kicondialog.cpp
  kfile/krecentdirs.cpp
  kfile/kdiskfreesp.cpp
  kfile/kimagefilepreview.cpp
  kfile/kfilesharedialog.cpp
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
  kfile/kfilespeedbar.cpp
  kfile/kpreviewwidgetbase.cpp
  kfile/kfilemetapreview.cpp
  kfile/kencodingfiledialog.cpp
  kfile/kacleditwidget.cpp
  kfile/kfile.cpp
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
  kssl/ksslinfodialog.cc
  kssl/ksslcertdialog.cc
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

set(kio_LIB_PRIVATE_SRCS
    kio/kfileshare_p.h
)

kde4_add_ui_files(kio_LIB_SRCS
   kfile/kpropertiesdesktopadvbase.ui
   kfile/kpropertiesdesktopbase.ui
   kssl/keygenwizard.ui
   kssl/keygenwizard2.ui
)

kde4_automoc(${kio_LIB_SRCS})
kde4_automoc(${kio_LIB_PRIVATE_SRCS})

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

set_target_properties(kio PROPERTIES VERSION 5.0.0 SOVERSION 5 )

########### install files ###############
install(TARGETS kio  DESTINATION ${LIB_INSTALL_DIR} )

install( PROGRAMS useragent.pl proxytype.pl  DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )


install( FILES magic  DESTINATION  ${MIME_INSTALL_DIR} )
install( FILES data.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES kpasswdserver.desktop  DESTINATION  ${SERVICES_INSTALL_DIR}/kded )
install( FILES
 application.desktop
 kcmodule.desktop
 kcmoduleinit.desktop
 kdatatool.desktop
 kfileplugin.desktop
 kscan.desktop
 kurifilterplugin.desktop
 renamedialogplugin.desktop
 kfile/kpropertiesdialogplugin.desktop  
DESTINATION  ${SERVICETYPES_INSTALL_DIR} )

install( FILES kioslave.upd  DESTINATION  ${DATA_INSTALL_DIR}/kconf_update )
install(  FILES
  kio/connection.h
  kio/slaveinterface.h
  kio/slave.h
  kio/slaveconfig.h
  kio/sessiondata.h
  kio/global.h
  kio/passworddialog.h
  kio/netaccess.h
  kio/job.h
  kio/scheduler.h
  kio/jobclasses.h
  kio/copyjob.h
  kio/deletejob.h
  kio/jobuidelegate.h
  kio/paste.h
  kio/slavebase.h
  kio/filejob.h
  kio/progressbase.h
  kio/defaultprogress.h
  kio/statusbarprogress.h
  kio/tcpslavebase.h
  kio/forwardingslavebase.h
  kio/observer.h
  kio/chmodjob.h
  kio/authinfo.h
  kio/ioslave_defaults.h
  kio/http_slave_defaults.h
  kio/previewjob.h
  kio/thumbcreator.h
  kio/metainfojob.h
  kio/davjob.h
  kio/directorysizejob.h
  kio/renamedialog.h
  kio/renamedialogplugin.h
  kio/skipdialog.h
  kio/safesite.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kio)

install( FILES
  kio/kmimetype.h
  kio/kmimetypetrader.h
  kio/kimageio.h
  kio/krun.h
  kio/kdirwatch.h
  kio/kautomount.h
  kio/kar.h
  kio/ktar.h
  kio/kzip.h
  kio/kurifilter.h
  kio/kurlcompletion.h
  kio/kshellcompletion.h
  kio/kfileitem.h
  kio/kfileitemdelegate.h
  kio/kfileshare.h
  kio/ksambashare.h
  kio/knfsshare.h
  kio/kdirlister.h
  kio/kservicegroup.h
  kio/kdirnotify.h
  kio/kurlpixmapprovider.h
  kio/kprotocolmanager.h
  kio/kemailsettings.h
  kio/kscan.h
  kio/kdatatool.h
  kio/karchive.h
  kio/kfilefilter.h
  kio/kfilemetainfo.h
  kio/kmimetyperesolver.h
  kio/k3mimetyperesolver.h
  kio/kdbusservicestarter.h
  kio/kremoteencoding.h
  kio/kmimetypechooser.h
  kio/kacl.h
  kio/kdedesktopmimetype.h
  kio/kbuildsycocaprogressdialog.h
  kio/kdirmodel.h
  kfile/kcombiview.h
  kfile/kcustommenueditor.h
  kfile/kdiroperator.h
  kfile/kdirselectdialog.h
  kfile/kdiskfreesp.h
  kfile/kencodingfiledialog.h
  kfile/kfile.h
  kfile/kfiledetailview.h
  kfile/kfiledialog.h
  kfile/kfileiconview.h
  kfile/kfilepreview.h
  kfile/kfilesharedialog.h
  kfile/kfiletreebranch.h
  kfile/kfiletreeview.h
  kfile/kfiletreeviewitem.h
  kfile/kfileview.h
  kfile/kfilefiltercombo.h
  kfile/kicondialog.h
  kfile/kimagefilepreview.h
  kfile/kopenwithdialog.h
  kfile/kpreviewwidgetbase.h
  kfile/kpropertiesdialog.h
  kfile/kpropsdialog.h
  kfile/krecentdocument.h
  kfile/kurlbar.h
  kfile/kurlcombobox.h
  kfile/kurlrequester.h
  kfile/kurlrequesterdialog.h
  DESTINATION ${INCLUDE_INSTALL_DIR})

install(FILES kio/org.kde.KDirNotify.xml DESTINATION ${DBUS_INTERFACES_DIR})
install(FILES kio/org.kde.KIO.UIServer.xml DESTINATION ${DBUS_INTERFACES_DIR})

