cmake_minimum_required(VERSION 2.8.7)

project(kcolorwidgets)

find_package(extra-cmake-modules 0.0.2 REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${EXTRA_CMAKE_MODULES_MODULE_PATH})

include(ECMVersion)

include_directories(
${KDE4_KDECORE_INCLUDES}
${CMAKE_CURRENT_SOURCE_DIR}/colors
${DBUSMENUQT_INCLUDE_DIR}
)

add_subdirectory(src)
add_subdirectory(tests)
