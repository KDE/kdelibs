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

add_library(KConfigGui ${libkconfiggui_SRCS})
generate_export_header(KConfigGui)

target_link_libraries(KConfigGui Qt5::Widgets KConfigCore)
set_target_properties(KConfigGui PROPERTIES VERSION   ${KCONFIG_VERSION_STRING}
                                            SOVERSION ${KCONFIG_SOVERSION}
)

install(TARGETS KConfigGui EXPORT KConfigTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kconfiggui_export.h
   kconfiggui.h
   kconfigskeleton.h
   kwindowconfig.h
   DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
