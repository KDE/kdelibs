include_directories(
 ${KDE4_KDECORE_INCLUDES}
 ${CMAKE_CURRENT_SOURCE_DIR}
 ${CMAKE_BINARY_SOURCE_DIR}
)

set(kguiaddons_LIB_SRCS
 colors/kcolorspaces.cpp
 colors/kcolorutils.cpp
 colors/kcolorscheme.cpp
 colors/kcolorcollection.cpp
 colors/kcolormimedata.cpp
 text/kwordwrap.cpp
)

add_library(kguiaddons ${kguiaddons_LIB_SRCS})

generate_export_header(kguiaddons)

target_link_libraries(kguiaddons ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})

install(TARGETS kguiaddons EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
 ${CMAKE_CURRENT_BINARY_DIR}/kguiaddons_export.h
 colors/kcolorutils.h
 colors/kcolorscheme.h
 colors/kcolorcollection.h
 colors/kcolormimedata.h
 text/kwordwrap.h

 DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
