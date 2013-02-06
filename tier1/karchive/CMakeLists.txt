cmake_minimum_required(VERSION 2.8.10.1)

project(karchive)

find_package(ECM 0.0.6 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

include(FeatureSummary)

find_package(Qt5Transitional REQUIRED MODULE COMPONENTS Core Test)

find_package(ZLIB)
set_package_properties(ZLIB PROPERTIES
  URL "http://www.zlib.net"
  DESCRIPTION "Support for gzip compressed files and data streams"
  PURPOSE "Required by the core KDE libraries and some criti cal kioslaves"
)

find_package(BZip2)
set_package_properties(BZip2 PROPERTIES
  URL "http://www.bzip.org"
  DESCRIPTION "Support for BZip2 compressed files and data streams"
  TYPE RECOMMENDED
  PURPOSE "Support for BZip2 compressed files and data streams"
)

find_package(LibLZMA)
set_package_properties(LibLZMA PROPERTIES
  URL "http://tukaani.org/xz/"
  DESCRIPTION "Support for xz compressed files and data streams"
  PURPOSE "Support for xz compressed files and data streams"
)

include_directories(
  ${QT_INCLUDES}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${ZLIB_INCLUDE_DIR}
)

include(ECMSetupVersion)

ecm_setup_version(5 0 0 VARIABLE_PREFIX KARCHIVE
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/karchive_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/karchiveConfigVersion.cmake")


add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)


# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/karchive")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_LIST_DIR}/karchiveConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/karchiveConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
  PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/karchive_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/karchiveConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/karchiveConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT karchiveTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE karchiveTargets.cmake NAMESPACE KF5:: )
