cmake_minimum_required(VERSION 2.8.12)

project(ApiDox)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)

add_subdirectory( common )
add_subdirectory( licenses )

