cmake_minimum_required(VERSION 2.8.11)

project(KNewStuff)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

set(REQUIRED_QT_VERSION "5.2")
find_package(Qt5 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED COMPONENTS DBus Network Widgets Xml)

# Not part of kdelibs
if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
find_package(KF5 CONFIG REQUIRED
  #dependencies, remove once depending on 3.0.0
  KGuiAddons
  KWidgetsAddons
  KDBusAddons
  KI18n
  KConfigWidgets
  KCompletion
  KService
  KWindowSystem

  #actual dependencies
  KI18n
  ItemViews
  KArchive
  KIconThemes
  KTextWidgets
  XmlGui
  Solid
  KBookmarks
  KIO)

endif()

include(FeatureSummary)
include(ECMSetupVersion)

ecm_setup_version(5.0.0 VARIABLE_PREFIX KNEWSTUFF
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/knewstuff_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KNewStuffConfigVersion.cmake")

add_subdirectory(data)
add_subdirectory(src)
add_subdirectory(tests)

set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KNewStuff")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KNewStuffConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KNewStuffConfig.cmake"
  INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KNewStuffConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KNewStuffConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KNewStuffTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KNewStuffTargets.cmake NAMESPACE KF5:: )

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/knewstuff_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
