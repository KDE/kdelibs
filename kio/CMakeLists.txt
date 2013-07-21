project(kio)

if(KDE_PLATFORM_FEATURE_BINARY_INCOMPATIBLE_FEATURE_REDUCTION)
   set(KIO_NO_SOLID TRUE)
   set(KIO_NO_NEPOMUK TRUE)
endif(KDE_PLATFORM_FEATURE_BINARY_INCOMPATIBLE_FEATURE_REDUCTION)

if(NOT HAVE_NEPOMUK)
   set(KIO_NO_NEPOMUK TRUE)
endif(NOT HAVE_NEPOMUK)

if(NOT STRIGI_FOUND)
   set(KIO_NO_STRIGI TRUE)
endif(NOT STRIGI_FOUND)

if(NOT SOPRANO_FOUND)
   set(KIO_NO_SOPRANO TRUE)
endif(NOT SOPRANO_FOUND)

if(OPENSSL_FOUND)
   set(KSSL_HAVE_SSL 1)
   include_directories(${OPENSSL_INCLUDE_DIR})
endif(OPENSSL_FOUND)

if(NOT KIO_NO_NEPOMUK)
    include_directories(
       ${SOPRANO_INCLUDE_DIR}
       ${CMAKE_SOURCE_DIR}/nepomuk/
       ${CMAKE_SOURCE_DIR}/nepomuk/core/
       ${CMAKE_SOURCE_DIR}/nepomuk/types/    
       ${CMAKE_SOURCE_DIR}/nepomuk/ui/
       ${CMAKE_SOURCE_DIR}/nepomuk/query/
    )
endif(NOT KIO_NO_NEPOMUK)

# This applies to all subdirs; let's put kio's dependencies (and own dirs) here, once and for all
include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/bookmarks
   ${CMAKE_CURRENT_SOURCE_DIR}/kio
   ${CMAKE_CURRENT_SOURCE_DIR}/kfile
   ${CMAKE_CURRENT_SOURCE_DIR}/kssl
   ${CMAKE_CURRENT_SOURCE_DIR}      # for including kio/kio_export.h
   ${CMAKE_CURRENT_BINARY_DIR}/kio
   ${CMAKE_CURRENT_BINARY_DIR}/kssl
   ${CMAKE_CURRENT_BINARY_DIR}      # e.g. for observer_stub.h
   ${KDE4_KDEUI_INCLUDES}
   ${KDE4_KDECORE_INCLUDES}/
   ${ZLIB_INCLUDE_DIR}

   )
if (NOT KIO_NO_STRIGI)
   include_directories(${STRIGI_INCLUDE_DIR})
   set(KIO_EXTRA_LIBS ${KIO_EXTRA_LIBS} ${STRIGI_STREAMANALYZER_LIBRARY} ${STRIGI_STREAMS_LIBRARY})
endif()

if (NOT KIO_NO_SOPRANO)
   include_directories(${SOPRANO_INCLUDE_DIR})
endif (NOT KIO_NO_SOPRANO)

if(NOT KIO_NO_SOLID)
   include_directories(${CMAKE_BINARY_DIR}/solid/)
   include_directories(${CMAKE_SOURCE_DIR}/solid/)
   set(KIO_EXTRA_LIBS ${KIO_EXTRA_LIBS} ${KDE4_SOLID_LIBS})
endif(NOT KIO_NO_SOLID)

configure_file(kio/config-kio.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kio/config-kio.h )

configure_file(kssl/ksslconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kssl/ksslconfig.h )
#macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/kssl/ksslconfig.h )

add_subdirectory( kssl/kssl )
add_subdirectory( kssl/kcm )
add_subdirectory( misc )
add_subdirectory( tests )
if(STRIGI_FOUND)
   add_subdirectory( kio/dummyanalyzers )
endif(STRIGI_FOUND)

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
  kio/joburlcache.cpp
  kio/kabstractfileitemactionplugin.cpp
  kio/kacl.cpp
  kio/kbuildsycocaprogressdialog.cpp
  kio/kdatatool.cpp
  kio/kdbusservicestarter.cpp
  kio/kdesktopfileactions.cpp
  kio/kdirlister.cpp
  kio/kdirmodel.cpp
  kio/kdirnotify.cpp
  kio/kdynamicjobtracker.cpp
  kio/kemailsettings.cpp
  kio/kfileitem.cpp
  kio/kfileitemactionplugin.cpp
  kio/kfileitemlistproperties.cpp
  kio/kfileitemactions.cpp
  kio/kfileitemdelegate.cpp
  kio/kfilemetainfo.cpp
  kio/kfilemetainfoitem.cpp
  kio/kfileshare.cpp
  kio/kfilewriteplugin.cpp
  kio/kimageio.cpp
  kio/kmimetypechooser.cpp
  kio/kprotocolmanager.cpp
  kio/kremoteencoding.cpp
  kio/krun.cpp
  kio/kscan.cpp
  kio/kshellcompletion.cpp
  kio/kurifilter.cpp
  kio/kurlcompletion.cpp
  kio/kurlpixmapprovider.cpp
  kio/metainfojob.cpp
  kio/netaccess.cpp
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
  kio/kpasswdserver.cpp
  kio/kpasswdserverloop.cpp
  kio/usernotificationhandler.cpp
  kio/clipboardupdater.cpp
)

if(NOT KDE_NO_DEPRECATED)
  set(kiocore_STAT_SRCS
    ${kiocore_STAT_SRCS}
    kio/kmimetyperesolver.cpp
    kio/passworddialog.cpp
    )
endif(NOT KDE_NO_DEPRECATED)

if(NOT KIO_NO_SOLID)
  set(kiocore_STAT_SRCS
    ${kiocore_STAT_SRCS}
    kio/kstatusbarofflineindicator.cpp
    )
endif(NOT KIO_NO_SOLID)

qt4_add_dbus_adaptor(kiocore_STAT_SRCS kio/org.kde.kio.FileUndoManager.xml fileundomanager_p.h KIO::FileUndoManagerPrivate fileundomanager_adaptor KIOFileUndoManagerAdaptor)

qt4_add_dbus_interface(kiocore_STAT_SRCS kio/org.kde.kuiserver.xml kuiserver_interface)

set_source_files_properties(kio/org.kde.KPasswdServer.xml
   PROPERTIES INCLUDE kio/authinfo.h
)
qt4_add_dbus_interface(kiocore_STAT_SRCS kio/org.kde.KPasswdServer.xml kpasswdserver_interface)
install(FILES kio/org.kde.KPasswdServer.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

if(UNIX)
   set(kiocore_STAT_SRCS ${kiocore_STAT_SRCS}
       kio/kautomount.cpp
       kio/knfsshare.cpp
       kio/ksambashare.cpp
       kio/ksambasharedata.cpp
   )
endif(UNIX)

if(WIN32)
   set(kiocore_STAT_SRCS ${kiocore_STAT_SRCS}
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
  bookmarks/kbookmarkimporter_opera.cc
  bookmarks/kbookmarkimporter_ie.cc
  bookmarks/kbookmarkimporter_ns.cc
  bookmarks/kbookmarkdombuilder.cc
  bookmarks/kbookmarkdialog.cc
)

if(NOT KDE_NO_DEPRECATED)
  set(kbookmarks_STAT_SRCS
    ${kbookmarks_STAT_SRCS}
    bookmarks/kbookmarkimporter_crash.cc
    )
endif(NOT KDE_NO_DEPRECATED)

set(kfile_STAT_SRCS
  kfile/kabstractfilemodule.cpp
  kfile/kacleditwidget.cpp
  kfile/kdiskfreespaceinfo.cpp
  kfile/kencodingfiledialog.cpp
  kfile/kfile.cpp
  kfile/kfiledialog.cpp
  kfile/kfilemetadataconfigurationwidget.cpp
  kfile/kfilemetadatawidget.cpp
  kfile/kfilemetainfowidget.cpp
  kfile/kfilemetapreview.cpp
  kfile/kicondialog.cpp
  kfile/kimagefilepreview.cpp
  kfile/kmetaprops.cpp
  kfile/knfotranslator.cpp
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

if(NOT KIO_NO_SOLID)
  set(kfile_STAT_SRCS
    ${kfile_STAT_SRCS}
    kfile/kdiskfreespace.cpp
    kfile/kdevicelistitem.cpp
    kfile/kdevicelistmodel.cpp
    )
endif(NOT KIO_NO_SOLID)

if(NOT KIO_NO_NEPOMUK)
  set(kfile_STAT_SRCS
    ${kfile_STAT_SRCS}
    kfile/kcommentwidget.cpp
    kfile/kfilemetadatareader.cpp
    kfile/kfilemetadataprovider.cpp
    )
endif(NOT KIO_NO_NEPOMUK)

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
  kssl/sslui.cpp
  kssl/ksslcertificatebox.cpp
  kssl/ksslinfodialog.cpp
# below is what's left of KDE2/3 SSL code, still required for certificate chain validation
# by hand and for key generation. Network code has been removed.
  kssl/kopenssl.cpp
  kssl/kssl.cpp
  kssl/ksslcertchain.cpp
  kssl/ksslcertificate.cpp
  kssl/ksslkeygen.cpp
  kssl/ksslsettings.cpp
  kssl/ksslutils.cpp
  kssl/ksslx509v3.cpp
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

if(NOT KIO_NO_NEPOMUK)
    set(kfilemetadatareaderprocess_SRCS
        kfile/kfilemetadatareaderprocess.cpp
    )
endif(NOT KIO_NO_NEPOMUK)

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

kde4_add_library(kio ${LIBRARY_TYPE} ${kio_LIB_SRCS})

if(NOT KIO_NO_NEPOMUK)
  kde4_add_executable(kfilemetadatareader ${kfilemetadatareaderprocess_SRCS})
  target_link_libraries(kfilemetadatareader ${KDE4_KIO_LIBS} ${KDE4_NEPOMUK_LIBRARY} nepomukutils)
  target_link_libraries(kio ${KDE4_NEPOMUK_LIBRARY} nepomukquery nepomukutils)
endif(NOT KIO_NO_NEPOMUK)
target_link_libraries(kio ${KDE4_KDEUI_LIBS} ${ZLIB_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTSVG_LIBRARY} ${X11_LIBRARIES} ${KIO_EXTRA_LIBS})
target_link_libraries(kio LINK_INTERFACE_LIBRARIES kdeui kdecore ${QT_QTNETWORK_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTGUI_LIBRARY} )

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
if(NOT KIO_NO_NEPOMUK)
    install(TARGETS kfilemetadatareader ${INSTALL_TARGETS_DEFAULT_ARGS})
endif(NOT KIO_NO_NEPOMUK)

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
 kio/kfileitemactionplugin.desktop
 kfile/kiofilemodule.desktop
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
  kssl/sslui.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kio COMPONENT Devel)

if( NOT WIN32 )
  install( FILES
    kfile/kfilesharedialog.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
endif( NOT WIN32 )

install( FILES
  kio/kabstractfileitemactionplugin.h
  kio/kacl.h
  kio/kautomount.h
  kio/kbuildsycocaprogressdialog.h
  kio/kdatatool.h
  kio/kdbusservicestarter.h
  kio/kdesktopfileactions.h
  kio/kdirlister.h
  kio/kdirmodel.h
  kio/kdirnotify.h
  kio/kemailsettings.h
  kio/kfileitem.h
  kio/kfileitemlistproperties.h
  kio/kfileitemactionplugin.h
  kio/kfileitemactions.h
  kio/kfileitemdelegate.h
  kio/kfilemetainfo.h
  kio/kfilemetainfoitem.h
  kio/kfileshare.h
  kio/kfilewriteplugin.h
  kio/kimageio.h
  kio/kmimetypechooser.h
  kio/knfsshare.h
  kio/kprotocolmanager.h
  kio/kremoteencoding.h
  kio/krun.h
  kio/ksambashare.h
  kio/ksambasharedata.h
  kio/kscan.h
  kio/kshellcompletion.h
  kio/kurifilter.h
  kio/kurlcompletion.h
  kio/kurlpixmapprovider.h
  kio/predicateproperties.h
  kfile/kabstractfilemodule.h
  kfile/kabstractfilewidget.h
  kfile/kdiskfreespaceinfo.h
  kfile/kencodingfiledialog.h
  kfile/kfile.h
  kfile/kfiledialog.h
  kfile/kicondialog.h
  kfile/kimagefilepreview.h
  kfile/kfilemetadataconfigurationwidget.h
  kfile/kfilemetadatawidget.h
  kfile/kopenwithdialog.h
  kfile/kpreviewwidgetbase.h
  kfile/kpropertiesdialog.h
  kfile/kpropsdialog.h
  kfile/krecentdirs.h
  kfile/krecentdocument.h
  kfile/kurlcombobox.h
  kfile/kurlrequester.h
  kfile/kurlrequesterdialog.h
  bookmarks/kbookmark.h
  bookmarks/kbookmarkexporter.h
  bookmarks/kbookmarkimporter.h
  bookmarks/kbookmarkmanager.h
  bookmarks/kbookmarkmenu.h
  bookmarks/kbookmarkimporter_opera.h
  bookmarks/kbookmarkimporter_ie.h
  bookmarks/kbookmarkimporter_ns.h
  bookmarks/kbookmarkdombuilder.h
  bookmarks/konqbookmarkmenu.h
  bookmarks/kbookmarkdialog.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

if(NOT KDE_NO_DEPRECATED)
install( FILES
  bookmarks/kbookmarkimporter_crash.h
  kfile/kdiskfreespace.h
  kio/kmimetyperesolver.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

install( FILES
  kio/passworddialog.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kio COMPONENT Devel)
endif(NOT KDE_NO_DEPRECATED)

if(NOT KIO_NO_SOLID)
install( FILES
  kfile/kdevicelistmodel.h
  kio/kstatusbarofflineindicator.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
endif(NOT KIO_NO_SOLID)

install(FILES
   kio/org.kde.KDirNotify.xml
   kio/org.kde.kio.FileUndoManager.xml
   DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})


install(FILES kio/accept-languages.codes DESTINATION ${CONFIG_INSTALL_DIR})
