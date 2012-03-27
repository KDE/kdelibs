include_directories(
 ${KDE4_KDECORE_INCLUDES}
 ${CMAKE_CURRENT_SOURCE_DIR}
 ${CMAKE_CURRENT_SOURCE_DIR}/colors
 ${DBUSMENUQT_INCLUDE_DIR}
 ${CMAKE_BINARY_DIR}/staging/kdeguiaddons/src/
)

set(kdeguiaddons_LIB_SRCS
 colors/kcolorspaces.cpp
 colors/kcolorutils.cpp
 colors/kcolorscheme.cpp
 colors/kcolorcollection.cpp
 colors/kcolormimedata.cpp
)

add_library(kdeguiaddons ${LIBRARY_TYPE} ${kdeguiaddons_LIB_SRCS})

generate_export_header(kdeguiaddons)

target_link_libraries(kdeguiaddons ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDEUI_EXTRA_LIBS})

install(TARGETS kdeguiaddons EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
 ${CMAKE_CURRENT_BINARY_DIR}/kdeguiaddons_export.h
 colors/kcolorutils.h
 colors/kcolorscheme.h
 colors/kcolorcollection.h
 colors/kcolormimedata.h

 DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
