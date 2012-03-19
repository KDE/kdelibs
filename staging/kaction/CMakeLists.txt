cmake_minimum_required(VERSION 2.8.5)

project(kactions)

find_package(extra-cmake-modules 0.0.2 REQUIRED)


include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${EXTRA_CMAKE_MODULES_MODULE_PATH})


add_subdirectory(src)
