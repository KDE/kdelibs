cmake_minimum_required(VERSION 2.8.12)

project(XmlGui)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(KF5 5.0 MODULE REQUIRED CMake Compiler InstallDirs)

set(QT_REQUIRED_VERSION 5.2.0)
find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets DBus Xml Network PrintSupport)

if(NOT WIN32)
    find_package(Qt5 ${QT_REQUIRED_VERSION} CONFIG REQUIRED X11Extras)

    find_package(X11)
    set_package_properties(X11 PROPERTIES TYPE OPTIONAL)
    set(HAVE_X11 ${X11_FOUND})
endif(NOT WIN32)

include(FeatureSummary)
include(ECMSetupVersion)

ecm_setup_version(5.0.0 VARIABLE_PREFIX XMLGUI
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/xmlgui_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/XmlGuiConfigVersion.cmake")

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
