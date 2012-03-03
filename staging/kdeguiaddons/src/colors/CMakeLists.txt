cmake_minimum_required(VERSION 2.8.7)

project(kdeguiaddons)

find_package(extra-cmake-modules 0.0.2 REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${EXTRA_CMAKE_MODULES_MODULE_PATH})

include(ECMVersion)

include_directories(
 ${KDE4_KDECORE_INCLUDES}
 ${CMAKE_CURRENT_SOURCE_DIR}/colors
 ${DBUSMENUQT_INCLUDE_DIR}
)

set(kdeguiaddons_LIB_SRCS
 kcolorspaces.cpp
 kcolorutils.cpp
 kcolorscheme.cpp
 kcolorcollection.cpp
 kcolorhelpers.cpp
 kcolormimedata.cpp
)

add_library(kdeguiaddons ${LIBRARY_TYPE} ${kdeguiaddons_LIB_SRCS})

generate_export_header(kdeguiaddons)

target_link_libraries(kdeguiaddons ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDEUI_EXTRA_LIBS})

install(TARGETS kdeguiaddons EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
 kcolorutils.h
 kcolorscheme.h
 kcolorcollection.h
 kcolorchoosermode.h
 kcolormimedata.h

 DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

add_subdirectory(tests)
