
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

set(kguiaddons_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/colors
  ${CMAKE_CURRENT_SOURCE_DIR}/text
  )
target_include_directories(kguiaddons PUBLIC "$<BUILD_INTERFACE:${kguiaddons_INCLUDES}>")
target_link_libraries(kguiaddons ${KConfig_LIBRARIES} Qt5::Widgets)
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
