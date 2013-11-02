project(kio)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_SOURCE_DIR}/cmake/modules) #TODO: remove cmake/modules when extra-cmake-modules is done

set(REQUIRED_QT_VERSION "5.2")
find_package(Qt5 "${REQUIRED_QT_VERSION}" CONFIG REQUIRED Widgets Network Svg Xml Test)
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

if(KDE_PLATFORM_FEATURE_BINARY_INCOMPATIBLE_FEATURE_REDUCTION)
   set(KIO_NO_SOLID TRUE)
endif(KDE_PLATFORM_FEATURE_BINARY_INCOMPATIBLE_FEATURE_REDUCTION)

if(NOT WIN32)
    find_package(Qt5 5.2.0 CONFIG REQUIRED X11Extras)
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
)

if(NOT KDE_NO_DEPRECATED)
  set(kiocore_STAT_SRCS
    ${kiocore_STAT_SRCS}
    )
endif(NOT KDE_NO_DEPRECATED)

if(NOT KIO_NO_SOLID)
  set(kiocore_STAT_SRCS
    ${kiocore_STAT_SRCS}
    )
endif(NOT KIO_NO_SOLID)

if(UNIX)
   set(kiocore_STAT_SRCS ${kiocore_STAT_SRCS}
   )
endif(UNIX)


########### next target ###############



set(kio_LIB_SRCS
   ${kiocore_STAT_SRCS}
)

add_library(kio ${kio_LIB_SRCS})
generate_export_header(kio)

set(kio_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/kio
   ${CMAKE_CURRENT_SOURCE_DIR}/kfile
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

target_link_libraries(kio PRIVATE KIOWidgets KIOCore KIOFileWidgets KF5::KCoreAddons KF5::KI18n KCompletion KJobWidgets KF5::KIconThemes XmlGui KF5::KWidgetsAddons KF5::KWindowSystem ${ZLIB_LIBRARY} Qt5::Network Qt5::Xml Qt5::Svg ${X11_LIBRARIES} ${KIO_EXTRA_LIBS} KWallet KCrash KTextWidgets Qt5::X11Extras)
target_link_libraries(kio PUBLIC KIOWidgets KIOCore Qt5::Network Qt5::Xml Qt5::Widgets KF5::KWidgetsAddons)

if(ACL_FOUND)
   target_link_libraries(kio PRIVATE ${ACL_LIBS})
endif(ACL_FOUND)

if(X11_Xrender_FOUND)
   target_link_libraries(kio PRIVATE ${X11_Xrender_LIB})
endif(X11_Xrender_FOUND)

set_target_properties(kio PROPERTIES VERSION ${KDE_NON_GENERIC_LIB_VERSION}
                                     SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION}
                     )

add_subdirectory( misc )

########### install files ###############
install(TARGETS kio EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
install( PROGRAMS useragent.pl proxytype.pl  DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )


install( FILES data.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )

install(  FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kio_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kio COMPONENT Devel)

install( FILES

  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

if(NOT KIO_NO_SOLID)
install( FILES
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
endif(NOT KIO_NO_SOLID)

