
set(kguiaddons_LIB_SRCS
 colors/kcolorspaces.cpp
 colors/kcolorutils.cpp
 colors/kcolorcollection.cpp
 colors/kcolormimedata.cpp
 text/kwordwrap.cpp
 fonts/kfontutils.cpp
 util/klocalimagecacheimpl.cpp
 util/kmodifierkeyinfo.cpp
 util/kmodifierkeyinfoprovider.cpp
 util/urlhandler.cpp
)

message("Port to Qt5 native filter")
#if (X11_FOUND AND X11_Xkb_FOUND)
#    include_directories (
#        ${X11_Xkb_INCLUDE_PATH}
#        ${X11_Xlib_INCLUDE_PATH}
#    )
#    set(kguiaddons_LIB_SRCS ${kguiaddons_LIB_SRCS} util/kmodifierkeyinfoprovider_x11.cpp)
#else (X11_FOUND AND X11_Xkb_FOUND)
    set (kguiaddons_LIB_SRCS ${kguiaddons_LIB_SRCS} util/kmodifierkeyinfoprovider_dummy.cpp)
#endif (X11_FOUND AND X11_Xkb_FOUND)

add_library(KGuiAddons ${kguiaddons_LIB_SRCS})

generate_export_header(KGuiAddons)

set(kguiaddons_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/colors
  ${CMAKE_CURRENT_SOURCE_DIR}/fonts
  ${CMAKE_CURRENT_SOURCE_DIR}/text
  ${CMAKE_CURRENT_SOURCE_DIR}/util
  )
target_include_directories(KGuiAddons PUBLIC "$<BUILD_INTERFACE:${kguiaddons_INCLUDES}>")
target_link_libraries(KGuiAddons Qt5::Gui)
set_target_properties(KGuiAddons PROPERTIES VERSION   ${KGUIADDONS_VERSION_STRING}
                                            SOVERSION ${KGUIADDONS_SOVERSION} )


install(TARGETS KGuiAddons EXPORT KGuiAddonsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
 ${CMAKE_CURRENT_BINARY_DIR}/kguiaddons_export.h
 colors/kcolorutils.h
 colors/kcolorcollection.h
 colors/kcolormimedata.h
 text/kwordwrap.h
 fonts/kfontutils.h
 util/kimagecache.h
 util/klocalimagecacheimpl.cpp
 util/kmodifierkeyinfo.h
 DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
