include_directories(
 ${KDE4_KDECORE_INCLUDES}
 ${CMAKE_CURRENT_SOURCE_DIR}
 ${CMAKE_BINARY_SOURCE_DIR}
)

set(kdeguiaddons_LIB_SRCS
 colors/kcolorspaces.cpp
 colors/kcolorutils.cpp
 colors/kcolorscheme.cpp
 colors/kcolorcollection.cpp
 colors/kcolormimedata.cpp
 text/kwordwrap.cpp
)

add_library(kdeguiaddons ${LIBRARY_TYPE} ${kdeguiaddons_LIB_SRCS})

generate_export_header(kdeguiaddons)

target_link_libraries(kdeguiaddons ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})

install(TARGETS kdeguiaddons EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
 ${CMAKE_CURRENT_BINARY_DIR}/kdeguiaddons_export.h
 colors/kcolorutils.h
 colors/kcolorscheme.h
 colors/kcolorcollection.h
 colors/kcolormimedata.h
 text/kwordwrap.h

 DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
