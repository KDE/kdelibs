cmake_minimum_required(VERSION 2.8.5)

project(threadweaver)

find_package(extra-cmake-modules 0.0.2 REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${EXTRA_CMAKE_MODULES_MODULE_PATH})

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

include(ECMQtFramework)

find_package(Qt5Transitional REQUIRED Core Test)

add_subdirectory(Weaver)
# disabled for default kdelibs build:
# add_subdirectory(WeaverGui)
# add_subdirectory(Experimental)
 add_subdirectory(Tests)
# add_subdirectory(Examples)

