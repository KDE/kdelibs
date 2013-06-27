
set(kguiaddons_LIB_SRCS
 colors/kcolorspaces.cpp
 colors/kcolorutils.cpp
 colors/kcolorcollection.cpp
 colors/kcolormimedata.cpp
 text/kwordwrap.cpp
 fonts/kfontutils.cpp
 util/kmodifierkeyinfo.cpp
 util/kmodifierkeyinfoprovider.cpp
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

add_library(kguiaddons ${kguiaddons_LIB_SRCS})

generate_export_header(kguiaddons)

set(kguiaddons_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/colors
  ${CMAKE_CURRENT_SOURCE_DIR}/text
  )
target_include_directories(kguiaddons PUBLIC "$<BUILD_INTERFACE:${kguiaddons_INCLUDES}>")
target_link_libraries(kguiaddons Qt5::Gui)
set_target_properties(kguiaddons PROPERTIES VERSION   ${KGUIADDONS_VERSION_STRING}
                                            SOVERSION ${KGUIADDONS_SOVERSION} )


install(TARGETS kguiaddons EXPORT kguiaddonsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
 ${CMAKE_CURRENT_BINARY_DIR}/kguiaddons_export.h
 colors/kcolorutils.h
 colors/kcolorcollection.h
 colors/kcolormimedata.h
 text/kwordwrap.h
 fonts/kfontutils.h
 util/kmodifierkeyinfo.h
 DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
