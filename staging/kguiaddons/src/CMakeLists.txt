include_directories(
 ${CMAKE_CURRENT_SOURCE_DIR}
 ${CMAKE_BINARY_SOURCE_DIR}
 ${kconfig_BINARY_DIR}/src/core
 ${kconfig_SOURCE_DIR}/src/core
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

target_link_libraries(kguiaddons kconfigcore ${kdeqt5staging_LIBRARIES} Qt5::Widgets)
set_target_properties(kguiaddons PROPERTIES VERSION   ${KGUIADDONS_VERSION_STRING}
                                            SOVERSION ${KGUIADDONS_SOVERSION} )


install(TARGETS kguiaddons EXPORT kguiaddonsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
 ${CMAKE_CURRENT_BINARY_DIR}/kguiaddons_export.h
 colors/kcolorutils.h
 colors/kcolorscheme.h
 colors/kcolorcollection.h
 colors/kcolormimedata.h
 text/kwordwrap.h

 DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
