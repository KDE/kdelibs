cmake_minimum_required(VERSION 2.8.7)

project(kdecolorwidgets)

find_package(extra-cmake-modules 0.0.2 REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${EXTRA_CMAKE_MODULES_MODULE_PATH})

include(ECMVersion)

include_directories(
 ${KDE4_KDECORE_INCLUDES}
 ${CMAKE_CURRENT_SOURCE_DIR}/colors
 ${DBUSMENUQT_INCLUDE_DIR}
)

set(kdecolorwidgets_LIB_SRCS
 kcolorbutton.cpp
 kcolorcombo.cpp
 kcolordialog.cpp
 kcolorvalueselector.cpp
 khuesaturationselect.cpp
)

add_library(kdecolorwidgets ${LIBRARY_TYPE} ${kdecolorwidgets_LIB_SRCS})

generate_export_header(kdecolorwidgets)

target_link_libraries(kdecolorwidgets kdeguiaddons ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDEUI_EXTRA_LIBS})

install(TARGETS kdecolorwidgets EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
 ${CMAKE_CURRENT_BINARY_DIR}/kcolorwidgets_export.h
 kcolorbutton.h
 kcolorcombo.h
 kcolordialog.h
 kcolorvalueselector.h
 khuesaturationselect.h

 DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

install( FILES 40.colors Oxygen.colors Web.colors
		Royal.colors Rainbow.colors DESTINATION ${CONFIG_INSTALL_DIR}/colors)

