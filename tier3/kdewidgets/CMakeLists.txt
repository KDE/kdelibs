cmake_minimum_required(VERSION 2.8.12)

project(KDEWidgets)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(KF5 5.0 MODULE REQUIRED CMake Compiler InstallDirs)
set(REQUIRED_QT_VERSION 5.2.0)
find_package(Qt5Core ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KDEWidgets")

add_subdirectory(src)
add_subdirectory(tests)

install( FILES
    KDEWidgetsMacros.cmake KDEWidgetsConfig.cmake
    DESTINATION ${CMAKECONFIG_INSTALL_DIR} COMPONENT Devel )

install(EXPORT KDEWidgetsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KDEWidgetsTargets.cmake NAMESPACE KF5:: )

if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
