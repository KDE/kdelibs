
cmake_minimum_required(VERSION 2.8.10.1)

project(kio)

find_package(ECM 0.0.7 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake )

find_package(KF5 5.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

include(ECMQtFramework)
include(ECMWriteVersionHeader)

# TODO: Remove these
remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_STRICT_ITERATORS)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_KEYWORDS)
remove_definitions(-DQT_USE_FAST_CONCATENATION)
remove_definitions(-DQT_USE_FAST_OPERATOR_PLUS)

find_package(Qt5Transitional REQUIRED MODULE COMPONENTS DBus Widgets Test)

ecm_write_version_header(${CMAKE_CURRENT_BINARY_DIR}/src/kio_version.h)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/src/kio_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

include_directories(
  ${QT_INCLUDES}
  ${CMAKE_BINARY_DIR}/kdecore
  ${CMAKE_SOURCE_DIR}/kdecore/localization # i18n
)

add_subdirectory(src)
#add_subdirectory(autotests)
#add_subdirectory(tests)
