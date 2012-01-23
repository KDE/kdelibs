cmake_minimum_required(VERSION 2.8.5)

project(kplotting)

find_package(extra-cmake-modules 0.0.2 REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${EXTRA_CMAKE_MODULES_MODULE_PATH})

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

include(ECMQtFramework)

find_package(Qt4 REQUIRED QtCore QtGui)

include(${QT_USE_FILE})

add_subdirectory(src)
add_subdirectory(examples)

include(FeatureSummary)
feature_summary(WHAT ENABLED_FEATURES)
