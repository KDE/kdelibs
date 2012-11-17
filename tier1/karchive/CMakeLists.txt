cmake_minimum_required(VERSION 2.8.9)

project(karchive)

#find_package(ECM 0.0.5 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_AUTOMOC ON)
enable_testing()

include(FeatureSummary)
include(GenerateExportHeader)

# Qt libs and are flexible regarding the install location of Qt under Windows
set(QT_USE_IMPORTED_TARGETS TRUE)
find_package(Qt4 REQUIRED)

find_package(ZLIB)
set_package_properties(ZLIB PROPERTIES
  URL "http://www.zlib.net"
  DESCRIPTION "Support for gzip compressed files and data streams"
  PURPOSE "Required by the core KDE libraries and some criti cal kioslaves"
)

include_directories(
  ${QT_INCLUDES}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${ZLIB_INCLUDE_DIR}
)

#ecm_write_version_header(${CMAKE_CURRENT_BINARY_DIR}/karchive_version.h)

set(INCLUDE_INSTALL_DIR "include")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/karchive_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)

feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
