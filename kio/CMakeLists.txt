project(kio)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules) #TODO: remove cmake/modules when extra-cmake-modules is done

set(REQUIRED_QT_VERSION "5.2")
find_package(Qt5 "${REQUIRED_QT_VERSION}" REQUIRED NO_MODULE COMPONENTS Widgets Network Svg Xml Test)
find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

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
set(HAVE_X11 ${X11_FOUND})
if(X11_FOUND)
    set_package_properties(X11_Xrender PROPERTIES DESCRIPTION "X Rendering Extension (libXrender)"
                           URL "http://www.x.org" TYPE RECOMMENDED
                           PURPOSE "Support for compositing, rendering operations, and alpha-blending")
    set(HAVE_XRENDER ${X11_Xrender_FOUND})
endif(X11_FOUND)

find_package(ACL)
set(HAVE_POSIX_ACL ${ACL_FOUND})
set(HAVE_LIBACL ${ACL_FOUND})

set_package_properties(ACL PROPERTIES DESCRIPTION "LibACL" URL "ftp://oss.sgi.com/projects/xfs/cmd_tars"
                       TYPE RECOMMENDED PURPOSE "Support for manipulating access control lists")

include(ConfigureChecks.cmake)

configure_file(kio/config-kio.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kio/config-kio.h )

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
  kfile/kencodingfiledialog.cpp
  kfile/kfiledialog.cpp
  kfile/kimagefilepreview.cpp
  kfile/kpreviewwidgetbase.cpp
  kfile/krecentdirs.cpp
  kfile/kfilemetapreview.cpp
)

if(NOT KIO_NO_SOLID)
  set(kfile_STAT_SRCS
    ${kfile_STAT_SRCS}
    kfile/kdevicelistitem.cpp
    kfile/kdevicelistmodel.cpp
    )
endif(NOT KIO_NO_SOLID)


########### next target ###############



set(kio_LIB_SRCS
   ${kiocore_STAT_SRCS}
   ${kfile_STAT_SRCS}
)

add_library(kio ${kio_LIB_SRCS})
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
