cmake_minimum_required(VERSION 2.8.8)

project(inqt5)

find_package(extra-cmake-modules 0.0.2 REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${EXTRA_CMAKE_MODULES_MODULE_PATH})

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

include(ECMQtFramework)

find_package(Qt4 REQUIRED QtCore)

include(${QT_USE_FILE})

include(FeatureSummary)

# where to put the exports, in case we are not build as part as the whole kdelibs
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(inqt5_EXPORT_NAME inqt5)
else ()
  set(inqt5_EXPORT_NAME kdelibs)
endif ()

add_subdirectory(src)
add_subdirectory(autotests)

