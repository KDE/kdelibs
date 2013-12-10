cmake_minimum_required(VERSION 2.8.12)

project(KInit)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

set(REQUIRED_QT_VERSION "5.2")
find_package(Qt5 "${REQUIRED_QT_VERSION}" CONFIG REQUIRED Core Gui Widgets DBus)
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)


ecm_setup_version(5.0.0 VARIABLE_PREFIX KINIT
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kinit_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KInitConfigVersion.cmake")

########### Configuring (mainly for proctitle) ###########
include(ConfigureChecks.cmake)

#optional features
find_package(X11)
set(HAVE_X11 ${X11_FOUND})

# used by 4 executables in this module
if(HAVE_SOCKET_LIBRARY)
  set(KINIT_SOCKET_LIBRARY socket)
else()
  set(KINIT_SOCKET_LIBRARY)
endif()

if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  find_package(KService ${KF5_VERSION} REQUIRED)
  find_package(KIO ${KF5_VERSION} REQUIRED)
  find_package(KF5I18n ${KF5_VERSION} REQUIRED)
  find_package(KF5WindowSystem ${KF5_VERSION} REQUIRED)
  find_package(KF5Crash ${KF5_VERSION} REQUIRED)

  # Due to a CMake bug, we need to explicitly find private dependencies of our dependencies
  # Remove when we depend on CMake 3.0.0
endif()

set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KInit")
configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KInitConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KInitConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

add_subdirectory(src)

install( FILES
	"${CMAKE_CURRENT_BINARY_DIR}/KInitConfig.cmake"
    KInitMacros.cmake ${CMAKE_CURRENT_BINARY_DIR}/KInitConfigVersion.cmake
    DESTINATION ${CMAKECONFIG_INSTALL_DIR} COMPONENT Devel )

install(FILES
        kde5init_dummy.cpp.in
        kde5init_win32lib_dummy.cpp.in

        DESTINATION ${CMAKECONFIG_INSTALL_DIR} COMPONENT Devel)
