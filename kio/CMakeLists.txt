project(kio)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

if(KDE_PLATFORM_FEATURE_BINARY_INCOMPATIBLE_FEATURE_REDUCTION)
   set(KIO_NO_SOLID TRUE)
endif(KDE_PLATFORM_FEATURE_BINARY_INCOMPATIBLE_FEATURE_REDUCTION)

if(NOT WIN32)
    find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS X11Extras)
endif(NOT WIN32)


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

set(kiocore_STAT_SRCS
  kio/defaultviewadapter.cpp
  kio/kdbusservicestarter.cpp
  kio/kimageio.cpp
)

if(NOT KDE_NO_DEPRECATED)
  set(kiocore_STAT_SRCS
    ${kiocore_STAT_SRCS}
    )
endif(NOT KDE_NO_DEPRECATED)

if(NOT KIO_NO_SOLID)
  set(kiocore_STAT_SRCS
    ${kiocore_STAT_SRCS}
    kio/kstatusbarofflineindicator.cpp
    )
endif(NOT KIO_NO_SOLID)

if(UNIX)
   set(kiocore_STAT_SRCS ${kiocore_STAT_SRCS}
   )
endif(UNIX)

set(kfile_STAT_SRCS
  kfile/kabstractfilemodule.cpp
  kfile/kacleditwidget.cpp
  kfile/kencodingfiledialog.cpp
  kfile/kfiledialog.cpp
  kfile/kimagefilepreview.cpp
  kfile/kpreviewwidgetbase.cpp
  kfile/kpropertiesdialog.cpp
  kfile/krecentdirs.cpp
  kfile/getgrouplist-fake.c
  kfile/kfilemetapreview.cpp
)

if(NOT KIO_NO_SOLID)
  set(kfile_STAT_SRCS
    ${kfile_STAT_SRCS}
    kfile/kdevicelistitem.cpp
    kfile/kdevicelistmodel.cpp
    )
endif(NOT KIO_NO_SOLID)

qt5_add_resources(kfile_STAT_SRCS kfile/kacleditwidget.qrc)


########### next target ###############



set(kio_LIB_SRCS
   ${kiocore_STAT_SRCS}
   ${kfile_STAT_SRCS}
)

kde4_add_ui_files(kio_LIB_SRCS
   kfile/kpropertiesdesktopadvbase.ui
   kfile/kpropertiesdesktopbase.ui
)

add_library(kio ${LIBRARY_TYPE} ${kio_LIB_SRCS})
generate_export_header(kio)

set(kio_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/kio
   ${CMAKE_CURRENT_SOURCE_DIR}/kfile
   ${CMAKE_CURRENT_SOURCE_DIR}/httpfilter
   ${CMAKE_CURRENT_BINARY_DIR}/..      # for including kio/kio_export.h

   ${CMAKE_CURRENT_BINARY_DIR}/kio
   ${CMAKE_CURRENT_BINARY_DIR}/kfile
   ${CMAKE_CURRENT_BINARY_DIR}      # e.g. for observer_stub.h
   ${CMAKE_SOURCE_DIR}/interfaces
)
target_include_directories(kio PUBLIC "$<BUILD_INTERFACE:${kio_INCLUDES}>" )

target_include_directories(kio PRIVATE
   ${ZLIB_INCLUDE_DIR}
   ${kiocore_BINARY_DIR}/..   # kio_version.h
)

target_link_libraries(kio KIOWidgets KIOCore ${KCoreAddons_LIBRARIES} KI18n KCompletion KJobWidgets KIconThemes XmlGui ${KWidgetsAddons_LIBRARIES} ${KWindowSystem_LIBRARIES} ${ZLIB_LIBRARY} Qt5::Network Qt5::Xml Qt5::Svg ${X11_LIBRARIES} ${KIO_EXTRA_LIBS} KWallet KCrash KTextWidgets Qt5::X11Extras)
target_link_libraries(kio LINK_INTERFACE_LIBRARIES KIOWidgets KIOCore Qt5::Network Qt5::Xml Qt5::Widgets KWidgetsAddons)

if(ACL_FOUND)
   target_link_libraries(kio ${ACL_LIBS})
endif(ACL_FOUND)

if(X11_Xrender_FOUND)
   target_link_libraries(kio ${X11_Xrender_LIB})
endif(X11_Xrender_FOUND)

set_target_properties(kio PROPERTIES VERSION ${KDE_NON_GENERIC_LIB_VERSION}
                                     SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION}
                     )

add_subdirectory( misc )
add_subdirectory( tests )
add_subdirectory( httpfilter/autotests )

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
 kurifilterplugin.desktop
 kfile/kpropertiesdialogplugin.desktop
 kfile/kiofilemodule.desktop
DESTINATION  ${SERVICETYPES_INSTALL_DIR} )

install(  FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kio_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kio COMPONENT Devel)

install( FILES
  kio/kdbusservicestarter.h
  kio/kimageio.h
  kfile/kabstractfilemodule.h
  kfile/kabstractfilewidget.h
  kfile/kencodingfiledialog.h
  kfile/kfiledialog.h
  kfile/kimagefilepreview.h
  kfile/kpreviewwidgetbase.h
  kfile/kpropertiesdialog.h
  kfile/kpropsdialog.h
  kfile/krecentdirs.h

  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

if(NOT KIO_NO_SOLID)
install( FILES
  kfile/kdevicelistmodel.h
  kio/kstatusbarofflineindicator.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
endif(NOT KIO_NO_SOLID)

install(FILES kio/accept-languages.codes DESTINATION ${CONFIG_INSTALL_DIR})
