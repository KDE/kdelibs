cmake_minimum_required(VERSION 2.8.12)

project(XmlGui)

# ECM setup
find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(ECMMarkNonGuiExecutable)
include(ECMSetupVersion)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

set(KF5_VERSION "5.0.0")

ecm_setup_version(${KF5_VERSION}
                  VARIABLE_PREFIX XMLGUI
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/xmlgui_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/XmlGuiConfigVersion.cmake")

# Dependencies
set(QT_REQUIRED_VERSION 5.2.0)
find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets DBus Xml Network PrintSupport)

if(NOT WIN32)
    find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED X11Extras)

    find_package(X11)
    set_package_properties(X11 PROPERTIES TYPE OPTIONAL)
    set(HAVE_X11 ${X11_FOUND})
endif(NOT WIN32)

if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    find_package(ItemViews ${KF5_VERSION} REQUIRED)
    find_package(KConfig ${KF5_VERSION} REQUIRED)
    find_package(KConfigWidgets ${KF5_VERSION} REQUIRED)
    find_package(KI18n ${KF5_VERSION} REQUIRED)
    find_package(KIconThemes ${KF5_VERSION} REQUIRED)
    find_package(KTextWidgets ${KF5_VERSION} REQUIRED)
    find_package(KWidgetsAddons ${KF5_VERSION} REQUIRED)
    find_package(KWindowSystem ${KF5_VERSION} REQUIRED)
endif()

# Subdirectories
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(autotests)


# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/XmlGui")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/XmlGuiConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/XmlGuiConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/XmlGuiConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/XmlGuiConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT XmlGuiTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE XmlGuiTargets.cmake NAMESPACE KF5:: )

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/xmlgui_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
