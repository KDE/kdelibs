cmake_minimum_required(VERSION 2.8.9)

project(karchive)

#find_package(ECM 0.0.5 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
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
  ${ZLIB_INCLUDE_DIR}
)

set(INCLUDE_INSTALL_DIR "include")

add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)

feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
