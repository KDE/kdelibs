cmake_minimum_required(VERSION 2.8.11)

project(KNewStuff)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings)

set(REQUIRED_QT_VERSION "5.2")
find_package(Qt5 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED COMPONENTS DBus Network Widgets Xml)

# Not part of kdelibs
set(KF5_VERSION "5.0.0")
if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
#dependencies, remove once depending on 3.0.0
find_package(KF5GuiAddons ${KF5_VERSION} REQUIRED)
find_package(KF5WidgetsAddons ${KF5_VERSION} REQUIRED)
find_package(KF5DBusAddons ${KF5_VERSION} REQUIRED)
find_package(KF5I18n ${KF5_VERSION} REQUIRED)
find_package(KF5ConfigWidgets ${KF5_VERSION} REQUIRED)
find_package(KF5Completion ${KF5_VERSION} REQUIRED)
find_package(KF5Service ${KF5_VERSION} REQUIRED)
find_package(KF5WindowSystem ${KF5_VERSION} REQUIRED)

#actual dependencies
find_package(KF5I18n ${KF5_VERSION} REQUIRED)
find_package(KF5ItemViews ${KF5_VERSION} REQUIRED)
find_package(KF5Archive ${KF5_VERSION} REQUIRED)
find_package(KF5IconThemes ${KF5_VERSION} REQUIRED)
find_package(KF5TextWidgets ${KF5_VERSION} REQUIRED)
find_package(KF5XmlGui ${KF5_VERSION} REQUIRED)
find_package(KF5Solid ${KF5_VERSION} REQUIRED)
find_package(KF5Bookmarks ${KF5_VERSION} REQUIRED)
find_package(KF5KIO ${KF5_VERSION} REQUIRED)

endif()

include(FeatureSummary)
include(ECMSetupVersion)

ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX KNEWSTUFF
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/knewstuff_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5NewStuffConfigVersion.cmake")

add_subdirectory(data)
add_subdirectory(src)
add_subdirectory(tests)

set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5NewStuff")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5NewStuffConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5NewStuffConfig.cmake"
  INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5NewStuffConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5NewStuffConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5NewStuffTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5NewStuffTargets.cmake NAMESPACE KF5:: )

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/knewstuff_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
