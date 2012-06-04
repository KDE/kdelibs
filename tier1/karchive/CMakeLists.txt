cmake_minimum_required(VERSION 2.8.8)

project(karchive)

find_package(ECM 0.0.4 REQUIRED NO_MODULE)

# hack: find_package(KF5) needs the FPHSA.cmake from cmake 2.8.8, but there is an older
#       one still in kdelibs/cmake/modules/.
#       OTOH some parts of karchive still use stuff from kdelibs/cmake/modules/, so we need
#       this dir too.
#       So remove it for find_package(KF5), and restore it afterwards again.
set(_CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${_CMAKE_MODULE_PATH})

include(ECMQtFramework)

find_package(Qt5Transitional REQUIRED Core)

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

add_subdirectory(src)
add_subdirectory(autotests)
add_subdirectory(tests)
