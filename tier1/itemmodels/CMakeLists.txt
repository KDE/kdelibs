cmake_minimum_required(VERSION 2.8)

project(itemmodels)

find_package(extra-cmake-modules REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${EXTRA_CMAKE_MODULES_MODULE_PATH})

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

include(ECMQtFramework)

find_package(Qt4 REQUIRED QtCore QtGui)

include(${QT_USE_FILE})

include(FeatureSummary)

add_subdirectory(src)
add_subdirectory(autotests)

install(FILES
  ${CMAKE_BINARY_DIR}/itemmodels_version.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
