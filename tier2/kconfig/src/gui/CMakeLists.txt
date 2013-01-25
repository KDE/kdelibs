include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../core
   ${CMAKE_CURRENT_BINARY_DIR}/../core
)

set(libkconfiggui_SRCS
   kconfiggui.cpp
   kconfiggroupgui.cpp
   kconfigskeleton.cpp
   kwindowconfig.cpp
)

add_library(kconfiggui ${libkconfiggui_SRCS})
generate_export_header(kconfiggui)

target_link_libraries(kconfiggui Qt5::Widgets kconfigcore)
set_target_properties(kconfiggui PROPERTIES VERSION   ${ECM_VERSION_STRING}
                                            SOVERSION ${ECM_SOVERSION}
)

install(TARGETS kconfiggui EXPORT kdelibsLibraryTargets ${ECM_TARGET_DEFAULT_ARGS})

install( FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kconfiggui_export.h
   kconfiggui.h
   kconfigskeleton.h
   kwindowconfig.h
   DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
