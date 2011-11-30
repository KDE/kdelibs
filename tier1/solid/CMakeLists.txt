
cmake_minimum_required(VERSION 2.8.6)

project(solid)

find_package(extra-cmake-modules REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${EXTRA_CMAKE_MODULES_MODULE_PATH})

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

include(ECMQtFramework)

# TODO: Remove these
remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_STRICT_ITERATORS)
remove_definitions(-DQT_NO_URL_CAST_FROM_STRING)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_KEYWORDS)
remove_definitions(-DQT_USE_FAST_CONCATENATION)
remove_definitions(-DQT_USE_FAST_OPERATOR_PLUS)


find_package(Qt4 REQUIRED QtCore QtGui)

include(${QT_USE_FILE})

include_directories (${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# Put this in QtFramework along with an if exists?
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

add_subdirectory( solid )
add_subdirectory( tests )
