project(kio)
macro_optional_find_package(FAM)

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
   ${KDE4_KDEUI_INCLUDES}
   ${CMAKE_SOURCE_DIR}/solid
   ${CMAKE_BINARY_DIR}/solid
   ${KDE4_KDECORE_INCLUDES}/
   ${STRIGI_INCLUDE_DIR}
   ${ZLIB_INCLUDE_DIR}
)

macro_bool_to_01(FAM_FOUND HAVE_FAM)

check_include_files(sys/inotify.h SYS_INOTIFY_H_FOUND)
macro_bool_to_01(SYS_INOTIFY_H_FOUND HAVE_SYS_INOTIFY_H)
if(WIN32)
 # currently for win32 only --> enable it for all in 4.1?
 option(USE_QFILESYSTEMWATCHER "Use QFileSystemWatcher instead polling for KDirWatch" ON)
 macro_bool_to_01(USE_QFILESYSTEMWATCHER HAVE_QFILESYSTEMWATCHER)
endif(WIN32)

configure_file(kio/config-kdirwatch.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kio/config-kdirwatch.h )
configure_file(kio/config-strigi.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kio/config-strigi.h )

configure_file(kssl/ksslconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kssl/ksslconfig.h )
#macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/kssl/ksslconfig.h )

add_subdirectory( kssl/kssl )
add_subdirectory( misc )
add_subdirectory( tests )
add_subdirectory( kfile/tests )
add_subdirectory( kio/dummyanalyzers )

set(kiocore_STAT_SRCS
  kio/accessmanager.cpp
  kio/accessmanagerreply_p.cpp
  kio/authinfo.cpp
  kio/chmodjob.cpp
  kio/connection.cpp
  kio/copyjob.cpp
  kio/dataprotocol.cpp
  kio/dataslave.cpp
  kio/davjob.cpp
  kio/defaultviewadapter.cpp
  kio/delegateanimationhandler.cpp
  kio/deletejob.cpp
  kio/directorysizejob.cpp
  kio/filejob.cpp
  kio/fileundomanager.cpp
  kio/forwardingslavebase.cpp
  kio/global.cpp
  kio/imagefilter.cpp
  kio/job.cpp
  kio/jobuidelegate.cpp
  kio/kacl.cpp
  kio/kar.cpp
  kio/karchive.cpp
  kio/kbuildsycocaprogressdialog.cpp
  kio/kdatatool.cpp
  kio/kdbusservicestarter.cpp
  kio/kdesktopfileactions.cpp
  kio/kdirlister.cpp
  kio/kdirmodel.cpp
  kio/kdirnotify.cpp
  kio/kdirwatch.cpp
  kio/kdynamicjobtracker.cpp
  kio/kemailsettings.cpp
  kio/kfileitem.cpp
  kio/kfileitemlistproperties.cpp
  kio/kfileitemactions.cpp
  kio/kfileitemdelegate.cpp
  kio/kfilemetainfo.cpp
  kio/kfilemetainfoitem.cpp
  kio/kfileshare.cpp
  kio/kfilewriteplugin.cpp
  kio/kimageio.cpp
  kio/klimitediodevice.cpp
  kio/kmimetypechooser.cpp
  kio/kmimetyperesolver.cpp
  kio/kmountpoint.cpp
  kio/kprotocolmanager.cpp
  kio/kremoteencoding.cpp
  kio/krun.cpp
  kio/kscan.cpp
  kio/kshellcompletion.cpp
  kio/kstatusbarofflineindicator.cpp
  kio/ktar.cpp
  kio/kurifilter.cpp
  kio/kurlcompletion.cpp
  kio/kurlpixmapprovider.cpp
  kio/kzip.cpp
  kio/metainfojob.cpp
  kio/netaccess.cpp
  kio/passworddialog.cpp
  kio/paste.cpp
  kio/pastedialog.cpp
  kio/predicateproperties.cpp
  kio/previewjob.cpp
  kio/renamedialog.cpp
  kio/renamedialogplugin.cpp
  kio/scheduler.cpp
  kio/sessiondata.cpp
  kio/skipdialog.cpp
  kio/slave.cpp
  kio/slavebase.cpp
  kio/slaveconfig.cpp
  kio/slaveinterface.cpp
  kio/tcpslavebase.cpp
  kio/thumbcreator.cpp
  kio/thumbsequencecreator.cpp
  kio/udsentry.cpp
  kio/hostinfo.cpp
)

qt4_add_dbus_adaptor(kiocore_STAT_SRCS kio/org.kde.kio.FileUndoManager.xml fileundomanager_p.h KIO::FileUndoManagerPrivate fileundomanager_adaptor KIOFileUndoManagerAdaptor)

if(UNIX)
   set(kiocore_STAT_SRCS ${kiocore_STAT_SRCS}
       kio/kautomount.cpp
       kio/knfsshare.cpp
       kio/ksambashare.cpp
   )
endif(UNIX)

if(WIN32)
   set(kiocore_STAT_SRCS ${kiocore_STAT_SRCS}
       kio/kdirwatch_win.cpp
       kio/krun_win.cpp
   )
endif(WIN32)

set(kbookmarks_STAT_SRCS
  bookmarks/kbookmark.cc
  bookmarks/kbookmarkimporter.cc
  bookmarks/kbookmarkmanager.cc
  bookmarks/kbookmarkmanageradaptor.cc
  bookmarks/kbookmarkmenu.cc
  bookmarks/konqbookmarkmenu.cc
  bookmarks/kbookmarkimporter_crash.cc
  bookmarks/kbookmarkimporter_opera.cc
  bookmarks/kbookmarkimporter_ie.cc
  bookmarks/kbookmarkimporter_ns.cc
  bookmarks/kbookmarkdombuilder.cc
  bookmarks/kbookmarkdialog.cc
)

set(kfile_STAT_SRCS
  kfile/kabstractfilemodule.cpp
  kfile/kacleditwidget.cpp
  kfile/kdevicelistitem.cpp
  kfile/kdevicelistmodel.cpp
  kfile/kdiskfreespace.cpp
  kfile/kdiskfreespaceinfo.cpp
  kfile/kencodingfiledialog.cpp
  kfile/kfile.cpp
  kfile/kfiledialog.cpp
  kfile/kfilemetainfowidget.cpp
  kfile/kfilemetapreview.cpp
  kfile/kicondialog.cpp
  kfile/kimagefilepreview.cpp
  kfile/kmetaprops.cpp
  kfile/kopenwithdialog.cpp
  kfile/kpreviewprops.cpp
  kfile/kpreviewwidgetbase.cpp
  kfile/kpropertiesdialog.cpp
  kfile/krecentdirs.cpp
  kfile/krecentdocument.cpp
  kfile/kurlcombobox.cpp
  kfile/kurlrequester.cpp
  kfile/kurlrequesterdialog.cpp
)
if(NOT WIN32)
  set(kfile_STAT_SRCS
    ${kfile_STAT_SRCS}
    kfile/kfilesharedialog.cpp
    kfile/kfsprocess.cpp
  )
endif(NOT WIN32)
qt4_add_resources(kfile_STAT_SRCS kfile/kacleditwidget.qrc)

# kio/kssl/Makefile.am: kssl

set(kssl_STAT_SRCS
  kssl/kssl.cpp
  kssl/ksslcertificatehome.cpp
  kssl/ksslsettings.cpp
  kssl/ksslcertificate.cpp
  kssl/ksslconnectioninfo.cpp
  kssl/ksslcertificatefactory.cpp
  kssl/ksslcertificatemanager.cpp
  kssl/ksslpeerinfo.cpp
  kssl/ksslinfodialog.cpp
  kssl/ksslcertificatebox.cpp
  kssl/ksslcertdialog.cpp
  kssl/ksslutils.cpp
  kssl/kopenssl.cpp
  kssl/ksslpkcs12.cpp
  kssl/ksslcertchain.cpp
  kssl/ksslkeygen.cpp
  kssl/ksslx509v3.cpp
  kssl/ksslsigners.cpp
  kssl/ksslpkcs7.cpp
  kssl/ksslpemcallback.cpp
  kssl/ksslx509map.cpp
  kssl/ksslsession.cpp
  kssl/ksmimecrypto.cpp
  kssl/ksslcsessioncache.cpp
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
    kio/krun_p.h
    kio/connection_p.h
    kio/slaveinterface_p.h
)

kde4_add_ui_files(kio_LIB_SRCS
   kfile/kpropertiesdesktopadvbase.ui
   kfile/kpropertiesdesktopbase.ui
   kssl/keygenwizard.ui
   kssl/keygenwizard2.ui
   kssl/sslinfo.ui
   kssl/certificateparty.ui
)

install(FILES kssl/ksslcertificatemanager.upd 
              kssl/ksslcertificatemanager.upd.sh
        DESTINATION  ${DATA_INSTALL_DIR}/kconf_update)

kde4_add_library(kio SHARED ${kio_LIB_SRCS})

target_link_libraries(kio ${KDE4_KDEUI_LIBS} ${ZLIB_LIBRARY} ${STRIGI_STREAMANALYZER_LIBRARY} ${STRIGI_STREAMS_LIBRARY} ${KDE4_SOLID_LIBS} ${QT_QTNETWORK_LIBRARY} ${QT_QTXML_LIBRARY} ${X11_LIBRARIES})
target_link_libraries(kio LINK_INTERFACE_LIBRARIES kdeui kdecore ${QT_QTNETWORK_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTGUI_LIBRARY} )

set(SYS_INOTIFY_H_FOUND 0)
if(SYS_INOTIFY_H_FOUND)
   macro_log_feature(FAM_FOUND "FAM" "File Alteration Monitor" "http://oss.sgi.com/projects/fam" FALSE "" "You have file alteration notification support built into your kernel, however you might consider installing FAM as it also supports NFS.")
else(SYS_INOTIFY_H_FOUND)
   macro_log_feature(FAM_FOUND "FAM" "File Alteration Monitor" "http://oss.sgi.com/projects/fam" FALSE "" "Provides file alteration notification facilities using a separate service.")
endif(SYS_INOTIFY_H_FOUND)

if(FAM_FOUND)
   include_directories(${FAM_INCLUDE_DIR})
   target_link_libraries(kio ${FAM_LIBRARIES})
endif(FAM_FOUND)

if(ACL_FOUND)
   target_link_libraries(kio ${ACL_LIBS})
endif(ACL_FOUND)

if(X11_Xrender_FOUND)
   target_link_libraries(kio ${X11_Xrender_LIB})
endif(X11_Xrender_FOUND)

set_target_properties(kio PROPERTIES VERSION ${KDE_NON_GENERIC_LIB_VERSION} 
                                     SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION} 
                     )


########### install files ###############
install(TARGETS kio EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( PROGRAMS useragent.pl proxytype.pl  DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )


install( FILES data.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )
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
 kio/kfilewrite.desktop
DESTINATION  ${SERVICETYPES_INSTALL_DIR} )

install( FILES kioslave.upd  DESTINATION  ${DATA_INSTALL_DIR}/kconf_update )
install(  FILES
  kio/kio_export.h
  kio/accessmanager.h
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
  kio/fileundomanager.h
  kio/tcpslavebase.h
  kio/forwardingslavebase.h
  kio/chmodjob.h
  kio/authinfo.h
  kio/ioslave_defaults.h
  kio/http_slave_defaults.h
  kio/previewjob.h
  kio/thumbcreator.h
  kio/thumbsequencecreator.h
  kio/metainfojob.h
  kio/davjob.h
  kio/directorysizejob.h
  kio/renamedialog.h
  kio/renamedialogplugin.h
  kio/skipdialog.h
  kio/udsentry.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kio COMPONENT Devel)

if( NOT WIN32 )
  install( FILES
    kfile/kfilesharedialog.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
endif( NOT WIN32 )

install( FILES
  kio/kacl.h
  kio/kar.h
  kio/karchive.h
  kio/kautomount.h
  kio/kbuildsycocaprogressdialog.h
  kio/kdatatool.h
  kio/kdbusservicestarter.h
  kio/kdesktopfileactions.h
  kio/kdirlister.h
  kio/kdirmodel.h
  kio/kdirnotify.h
  kio/kdirwatch.h
  kio/kemailsettings.h
  kio/kfileitem.h
  kio/kfileitemlistproperties.h
  kio/kfileitemactions.h
  kio/kfileitemdelegate.h
  kio/kfilemetainfo.h
  kio/kfilemetainfoitem.h
  kio/kfileshare.h
  kio/kfilewriteplugin.h
  kio/kimageio.h
  kio/kmimetypechooser.h
  kio/kmimetyperesolver.h
  kio/kmountpoint.h
  kio/knfsshare.h
  kio/kprotocolmanager.h
  kio/kremoteencoding.h
  kio/krun.h
  kio/ksambashare.h
  kio/kscan.h
  kio/kshellcompletion.h
  kio/kstatusbarofflineindicator.h
  kio/ktar.h
  kio/kurifilter.h
  kio/kurlcompletion.h
  kio/kurlpixmapprovider.h
  kio/kzip.h
  kio/predicateproperties.h
  kfile/kabstractfilemodule.h
  kfile/kabstractfilewidget.h
  kfile/kdevicelistmodel.h
  kfile/kdiskfreespace.h
  kfile/kdiskfreespaceinfo.h
  kfile/kencodingfiledialog.h
  kfile/kfile.h
  kfile/kfiledialog.h
  kfile/kicondialog.h
  kfile/kimagefilepreview.h
  kfile/kopenwithdialog.h
  kfile/kpreviewwidgetbase.h
  kfile/kpropertiesdialog.h
  kfile/kpropsdialog.h
  kfile/krecentdocument.h
  kfile/kurlcombobox.h
  kfile/kurlrequester.h
  kfile/kurlrequesterdialog.h
  bookmarks/kbookmark.h
  bookmarks/kbookmarkexporter.h
  bookmarks/kbookmarkimporter.h
  bookmarks/kbookmarkmanager.h
  bookmarks/kbookmarkmenu.h
  bookmarks/kbookmarkimporter_crash.h
  bookmarks/kbookmarkimporter_opera.h
  bookmarks/kbookmarkimporter_ie.h
  bookmarks/kbookmarkimporter_ns.h
  bookmarks/kbookmarkdombuilder.h
  bookmarks/konqbookmarkmenu.h
  bookmarks/kbookmarkdialog.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

install(FILES
   kio/org.kde.KDirNotify.xml
   kio/org.kde.kio.FileUndoManager.xml
   DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})


install(FILES kio/accept-languages.codes DESTINATION ${CONFIG_INSTALL_DIR})
