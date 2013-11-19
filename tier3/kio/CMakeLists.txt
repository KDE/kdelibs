cmake_minimum_required(VERSION 2.8.12)

project(KIO)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake )

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

include(FeatureSummary)

include(ECMSetupVersion)

ecm_setup_version(5.0.0 VARIABLE_PREFIX KIO
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/src/kio_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KIOConfigVersion.cmake")

if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  # Due to a CMake bug, we need to explicitly find private dependencies of our dependencies
  # Remove when we depend on CMake 3.0.0
  find_package(KDBusAddons ${KF5_VERSION} REQUIRED)
  find_package(KI18n ${KF5_VERSION} REQUIRED)
  find_package(KGuiAddons ${KF5_VERSION} REQUIRED)
  find_package(KConfigWidgets ${KF5_VERSION} REQUIRED)
  find_package(ItemViews ${KF5_VERSION} REQUIRED)
  find_package(KCoreAddons ${KF5_VERSION} REQUIRED)
  find_package(KWidgetsAddons ${KF5_VERSION} REQUIRED)
  find_package(KService ${KF5_VERSION} REQUIRED)
  find_package(KArchive ${KF5_VERSION} REQUIRED)
  find_package(Solid ${KF5_VERSION} REQUIRED)
  find_package(KJobWidgets ${KF5_VERSION} REQUIRED)
  find_package(KIconThemes ${KF5_VERSION} REQUIRED)
  find_package(KWindowSystem ${KF5_VERSION} REQUIRED)
  find_package(KCrash ${KF5_VERSION} REQUIRED)
  find_package(KDED ${KF5_VERSION} REQUIRED) # for org.kde.kded5.xml
  find_package(KCompletion ${KF5_VERSION} REQUIRED)
  find_package(KDocTools ${KF5_VERSION} REQUIRED)
  find_package(KTextWidgets ${KF5_VERSION} REQUIRED)
  find_package(XmlGui ${KF5_VERSION} REQUIRED)
  find_package(KBookmarks ${KF5_VERSION} REQUIRED)
  find_package(KNotifications ${KF5_VERSION} REQUIRED) #needed for kpac
else()
    set(_kdeBootStrapping TRUE)
    include(${CMAKE_SOURCE_DIR}/tier2/kdoctools/KDocToolsMacros.cmake)
    set(KDOCTOOLS_CUSTOMIZATION_DIR "${KDocTools_BINARY_DIR}/src/customization/")
endif()

# TODO: Remove these
remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

set(QT_REQUIRED_VERSION 5.2.0)
find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets DBus Network Concurrent Xml Test Script)

find_package(X11)
set(HAVE_X11 ${X11_FOUND})

add_subdirectory(docs)
add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KIO")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KIOConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KIOConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KIOConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KIOConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KIOTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KIOTargets.cmake NAMESPACE KF5:: )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/src/kio_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
