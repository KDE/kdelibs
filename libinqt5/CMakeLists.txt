cmake_minimum_required(VERSION 2.8.5)

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

add_subdirectory(src)
add_subdirectory(autotests)

