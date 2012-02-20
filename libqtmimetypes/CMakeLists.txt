cmake_minimum_required(VERSION 2.8.5)

project(QtMimeTypes)

find_package(extra-cmake-modules 0.0.2 REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${EXTRA_CMAKE_MODULES_MODULE_PATH})

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

include(ECMQtFramework)

find_package(Qt5Transitional REQUIRED Core)

include(FeatureSummary)

enable_testing() # should go into ECMQtFramework ?

# Qt itself isn't that strict, and frowns at Q_FOREACH
remove_definitions(-DQT_NO_KEYWORDS)


add_subdirectory(src)
add_subdirectory(examples)
