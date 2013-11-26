
set(kguiaddons_LIB_SRCS
 colors/kcolorspaces.cpp
 colors/kcolorutils.cpp
 colors/kcolorcollection.cpp
 colors/kcolormimedata.cpp
 text/kdatevalidator.cpp
 text/kwordwrap.cpp
 fonts/kfontutils.cpp
 util/kiconutils.cpp
 util/klocalimagecacheimpl.cpp
 util/kmodifierkeyinfo.cpp
 util/kmodifierkeyinfoprovider.cpp
 util/urlhandler.cpp
)

if (X11_FOUND AND X11_Xkb_FOUND AND XCB_XCB_FOUND)
    include_directories (
        ${X11_Xkb_INCLUDE_PATH}
        ${X11_Xlib_INCLUDE_PATH}
        ${XCB_XCB_INCLUDE_DIR}
        ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    )
    set(kguiaddons_LIB_SRCS ${kguiaddons_LIB_SRCS} util/kmodifierkeyinfoprovider_x11.cpp)
else ()
    set (kguiaddons_LIB_SRCS ${kguiaddons_LIB_SRCS} util/kmodifierkeyinfoprovider_dummy.cpp)
endif ()

add_library(KGuiAddons ${kguiaddons_LIB_SRCS})

generate_export_header(KGuiAddons)
add_library(KF5::KGuiAddons ALIAS KGuiAddons)

set(kguiaddons_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/colors
  ${CMAKE_CURRENT_SOURCE_DIR}/fonts
  ${CMAKE_CURRENT_SOURCE_DIR}/text
  ${CMAKE_CURRENT_SOURCE_DIR}/util
  )
target_include_directories(KGuiAddons PUBLIC "$<BUILD_INTERFACE:${kguiaddons_INCLUDES}>")
target_link_libraries(KGuiAddons PUBLIC Qt5::Gui)

if(X11_FOUND AND X11_Xkb_FOUND)
    target_link_libraries(KGuiAddons LINK_PRIVATE ${X11_LIBRARIES} ${XCB_XCB_LIBRARY})
endif()

set_target_properties(KGuiAddons PROPERTIES VERSION   ${KGUIADDONS_VERSION_STRING}
                                            SOVERSION ${KGUIADDONS_SOVERSION} )


install(TARGETS KGuiAddons EXPORT KGuiAddonsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
 ${CMAKE_CURRENT_BINARY_DIR}/kguiaddons_export.h
 colors/kcolorutils.h
 colors/kcolorcollection.h
 colors/kcolormimedata.h
 text/kdatevalidator.h
 text/kwordwrap.h
 fonts/kfontutils.h
 util/kiconutils.h
 util/kimagecache.h
 util/klocalimagecacheimpl.h
 util/kmodifierkeyinfo.h
 DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
