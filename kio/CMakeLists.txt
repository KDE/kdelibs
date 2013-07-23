project(kio)

add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

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

find_package(OpenSSL)
set_package_properties(OpenSSL PROPERTIES DESCRIPTION "Support for secure network communications (SSL and TLS)"
                       URL "http://openssl.org"
                       TYPE RECOMMENDED
                       PURPOSE "KDE uses OpenSSL for the bulk of secure communications, including secure web browsing via HTTPS"
                      )
if(OPENSSL_FOUND)
   set(KSSL_HAVE_SSL 1)
   include_directories(${OPENSSL_INCLUDE_DIR})
endif(OPENSSL_FOUND)

if(NOT KIO_NO_NEPOMUK)
    include_directories(
       ${SOPRANO_INCLUDE_DIR}
    )
endif(NOT KIO_NO_NEPOMUK)

if(NOT WIN32)
    find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS X11Extras)
endif(NOT WIN32)


if (NOT KIO_NO_STRIGI)
   include_directories(${STRIGI_INCLUDE_DIR})
   set(KIO_EXTRA_LIBS ${KIO_EXTRA_LIBS} ${STRIGI_STREAMANALYZER_LIBRARY} ${STRIGI_STREAMS_LIBRARY})
endif()

if (NOT KIO_NO_SOPRANO)
   include_directories(${SOPRANO_INCLUDE_DIR})
endif (NOT KIO_NO_SOPRANO)

if(NOT KIO_NO_SOLID)
   set(KIO_EXTRA_LIBS ${KIO_EXTRA_LIBS} ${KDE4_SOLID_LIBS})
endif(NOT KIO_NO_SOLID)

find_package(X11)
if(X11_FOUND)
	macro_bool_to_01(X11_FOUND HAVE_X11)
    set_package_properties(X11_Xrender PROPERTIES DESCRIPTION "X Rendering Extension (libXrender)"
                           URL "http://www.x.org" TYPE RECOMMENDED
                           PURPOSE "Support for compositing, rendering operations, and alpha-blending")
	macro_bool_to_01(X11_Xrender_FOUND HAVE_XRENDER)
endif(X11_FOUND)

find_package(ACL)
macro_bool_to_01(ACL_FOUND HAVE_LIBACL HAVE_POSIX_ACL)
set_package_properties(ACL PROPERTIES DESCRIPTION "LibACL" URL "ftp://oss.sgi.com/projects/xfs/cmd_tars"
                       TYPE RECOMMENDED PURPOSE "Support for manipulating access control lists")

include(ConfigureChecks.cmake)

configure_file(kio/config-kio.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kio/config-kio.h )

configure_file(kfile/config-kfile.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kfile/config-kfile.h )

configure_file(kssl/ksslconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kssl/ksslconfig.h )
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/kssl/ksslconfig.h )

set(kiocore_STAT_SRCS
  kio/accessmanager.cpp
  kio/accessmanagerreply_p.cpp
  kio/defaultviewadapter.cpp
  kio/delegateanimationhandler.cpp
  kio/fileundomanager.cpp
  kio/pixmaploader.cpp
  kio/imagefilter.cpp
  kio/jobuidelegate.cpp
  kio/joburlcache.cpp
  kio/kabstractfileitemactionplugin.cpp
  kio/kbuildsycocaprogressdialog.cpp
  kio/kdbusservicestarter.cpp
  kio/kdesktopfileactions.cpp
  kio/kdirlister.cpp
  kio/kdirmodel.cpp
  kio/kdynamicjobtracker.cpp
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
  kio/krun.cpp
  kio/kscan.cpp
  kio/kshellcompletion.cpp
  kio/kurifilter.cpp
  kio/kurlcompletion.cpp
  kio/kurlpixmapprovider.cpp
  kio/netaccess.cpp
  kio/paste.cpp
  kio/pastedialog.cpp
  kio/predicateproperties.cpp
  kio/previewjob.cpp
  kio/renamedialog.cpp
  kio/skipdialog.cpp
  kio/thumbcreator.cpp
  kio/thumbsequencecreator.cpp
)

if(NOT KDE_NO_DEPRECATED)
  set(kiocore_STAT_SRCS
    ${kiocore_STAT_SRCS}
    kio/passworddialog.cpp
    )
endif(NOT KDE_NO_DEPRECATED)

if(NOT KIO_NO_SOLID)
  set(kiocore_STAT_SRCS
    ${kiocore_STAT_SRCS}
    kio/kstatusbarofflineindicator.cpp
    )
endif(NOT KIO_NO_SOLID)

qt5_add_dbus_adaptor(kiocore_STAT_SRCS kio/org.kde.kio.FileUndoManager.xml fileundomanager_p.h KIO::FileUndoManagerPrivate fileundomanager_adaptor KIOFileUndoManagerAdaptor)

qt5_add_dbus_interface(kiocore_STAT_SRCS kio/org.kde.kuiserver.xml kuiserver_interface)

if(UNIX)
   set(kiocore_STAT_SRCS ${kiocore_STAT_SRCS}
       kio/kautomount.cpp
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

set(kfile_STAT_SRCS
  kfile/kabstractfilemodule.cpp
  kfile/kacleditwidget.cpp
  kfile/kencodingfiledialog.cpp
  kfile/kfile.cpp
  kfile/kfiledialog.cpp
  kfile/kfilemetadataconfigurationwidget.cpp
  kfile/kfilemetadatawidget.cpp
  kfile/kfilemetapreview.cpp
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
  kfile/getgrouplist-fake.c
)

if(NOT KIO_NO_SOLID)
  set(kfile_STAT_SRCS
    ${kfile_STAT_SRCS}
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

qt5_add_resources(kfile_STAT_SRCS kfile/kacleditwidget.qrc)

set(kssl_STAT_SRCS
  kssl/sslui.cpp
  kssl/ksslcertificatebox.cpp
  kssl/ksslinfodialog.cpp

  kssl/ksslcertificate.cpp # TODO: remove after porting khtml/java
  kssl/ksslcertchain.cpp # TODO: remove together with ksslcertificate.cpp
  kssl/ksslutils.cpp # TODO: remove together with ksslcertificate.cpp
  kssl/kssl.cpp # same
  kssl/ksslx509v3.cpp # same
)


########### next target ###############



set(kio_LIB_SRCS
   ${kiocore_STAT_SRCS}
   ${kbookmarks_STAT_SRCS}
   ${kfile_STAT_SRCS}
   ${kssl_STAT_SRCS}
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

kde4_add_library(kio ${LIBRARY_TYPE} ${kio_LIB_SRCS})
generate_export_header(kio)

set(kio_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/bookmarks
   ${CMAKE_CURRENT_SOURCE_DIR}/kio
   ${CMAKE_CURRENT_SOURCE_DIR}/kfile
   ${CMAKE_CURRENT_SOURCE_DIR}/kssl
   ${CMAKE_CURRENT_SOURCE_DIR}/httpfilter
   ${CMAKE_CURRENT_BINARY_DIR}/..      # for including kio/kio_export.h

   ${CMAKE_CURRENT_BINARY_DIR}/kio
   ${CMAKE_CURRENT_BINARY_DIR}/kssl
   ${CMAKE_CURRENT_BINARY_DIR}/kfile
   ${CMAKE_CURRENT_BINARY_DIR}      # e.g. for observer_stub.h
   ${CMAKE_SOURCE_DIR}/interfaces
)
target_include_directories(kio PUBLIC "$<BUILD_INTERFACE:${kio_INCLUDES}>" )

target_include_directories(kio PRIVATE
   ${ZLIB_INCLUDE_DIR}
   ${kiocore_BINARY_DIR}/..   # kio_version.h
)

if(NOT KIO_NO_NEPOMUK)
  kde4_add_executable(kfilemetadatareader ${kfilemetadatareaderprocess_SRCS})
  target_link_libraries(kfilemetadatareader ${KDE4_KIO_LIBS} ${KDE4_NEPOMUK_LIBRARY} nepomukutils ${KCoreAddons_LIBRARIES} KI18n KDE4Support)
  target_link_libraries(kio ${KDE4_NEPOMUK_LIBRARY} nepomukquery nepomukutils)
endif(NOT KIO_NO_NEPOMUK)

target_link_libraries(kio kdeqt5staging KIOCore ${KCoreAddons_LIBRARIES} KI18n KCompletion KJobWidgets ${KWidgetsAddons_LIBRARIES} ${KWindowSystem_LIBRARIES} ${KDE4_KDEUI_LIBS} ${ZLIB_LIBRARY} Qt5::Network Qt5::Xml Qt5::Svg ${X11_LIBRARIES} ${KIO_EXTRA_LIBS} KWallet KWidgets KCrash)
target_link_libraries(kio LINK_INTERFACE_LIBRARIES KIOCore kdeui kdecore Qt5::Network Qt5::Xml Qt5::Widgets )

if(ACL_FOUND)
   target_link_libraries(kio ${ACL_LIBS})
endif(ACL_FOUND)

if(X11_Xrender_FOUND)
   target_link_libraries(kio ${X11_Xrender_LIB})
endif(X11_Xrender_FOUND)

set_target_properties(kio PROPERTIES VERSION ${KDE_NON_GENERIC_LIB_VERSION}
                                     SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION}
                     )

add_subdirectory( kssl/kssl )
add_subdirectory( kssl/kcm )
add_subdirectory( misc )
add_subdirectory( tests )
add_subdirectory( httpfilter/autotests )
if (NOT KIO_NO_STRIGI)
   add_subdirectory( kio/dummyanalyzers )
endif()

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
 kfile/kpropertiesdialogplugin.desktop
 kio/kfilewrite.desktop
 kio/kfileitemactionplugin.desktop
 kfile/kiofilemodule.desktop
DESTINATION  ${SERVICETYPES_INSTALL_DIR} )

install(  FILES
  kio/accessmanager.h
  kio/pixmaploader.h
  kio/netaccess.h
  kio/jobuidelegate.h
  kio/paste.h
  kio/fileundomanager.h
  kio/previewjob.h
  kio/thumbcreator.h
  kio/thumbsequencecreator.h
  kio/renamedialog.h
  kio/skipdialog.h
  kssl/sslui.h
  ${CMAKE_CURRENT_BINARY_DIR}/kio_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kio COMPONENT Devel)

install( FILES
  kio/kabstractfileitemactionplugin.h
  kio/kautomount.h
  kio/kbuildsycocaprogressdialog.h
  kio/kdbusservicestarter.h
  kio/kdesktopfileactions.h
  kio/kdirlister.h
  kio/kdirmodel.h
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
  kio/krun.h
  kio/kscan.h
  kio/kshellcompletion.h
  kio/kurifilter.h
  kio/kurlcompletion.h
  kio/kurlpixmapprovider.h
  kio/predicateproperties.h
  kfile/kabstractfilemodule.h
  kfile/kabstractfilewidget.h
  kfile/kencodingfiledialog.h
  kfile/kfile.h
  kfile/kfiledialog.h
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
   kio/org.kde.kio.FileUndoManager.xml
   DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})


install(FILES kio/accept-languages.codes DESTINATION ${CONFIG_INSTALL_DIR})
