######## kidletime ####################

if (X11_XSync_FOUND AND XCB_XCB_FOUND AND X11_XCB_FOUND AND XCB_SYNC_FOUND)
   set (HAVE_XSYNC 1)
   include_directories( ${XCB_XCB_INCLUDE_DIR} ${XCB_SYNC_INCLUDE_DIR})
else ()
   set (HAVE_XSYNC 0)
endif ()

configure_file(config-kidletime.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kidletime.h )

set(kidletime_LIB_SRCS
    kidletime.cpp
    abstractsystempoller.cpp
    widgetbasedpoller.cpp # TODO: make optional, to avoid always depending on QtWidgets? Or port to QtGui?
)

set(kidletime_LIBS)

if (X11_FOUND)
   if (HAVE_XSCREENSAVER)
      set(kidletime_LIB_SRCS ${kidletime_LIB_SRCS} xscreensaverbasedpoller.cpp)
      qt5_add_dbus_interface(kidletime_LIB_SRCS org.freedesktop.ScreenSaver.xml screensaver_interface)
      set(kidletime_LIBS ${kidletime_LIBS} ${X11_Xscreensaver_LIB} Qt5::DBus)
   endif ()
   if (HAVE_XSYNC)
      set(kidletime_LIB_SRCS ${kidletime_LIB_SRCS} xsyncbasedpoller.cpp )
      set(kidletime_LIBS ${kidletime_LIBS} ${X11_Xext_LIB} ${XCB_XCB_LIBRARY} ${X11_XCB_LIBRARIES} ${XCB_SYNC_LIBRARY})
   endif ()
   set(kidletime_LIBS ${kidletime_LIBS} ${X11_LIBRARIES} Qt5::X11Extras)
elseif (APPLE)
   set(kidletime_LIB_SRCS ${kidletime_LIB_SRCS} macpoller.cpp)
elseif (WIN32)
   set(kidletime_LIB_SRCS ${kidletime_LIB_SRCS} windowspoller.cpp)
endif (X11_FOUND)

add_library(KIdleTime ${kidletime_LIB_SRCS})
generate_export_header(KIdleTime)
target_link_libraries(KIdleTime LINK_PUBLIC Qt5::Core)
target_link_libraries(KIdleTime LINK_PRIVATE Qt5::Widgets ${kidletime_LIBS})

set_target_properties(KIdleTime PROPERTIES  VERSION ${KIDLETIME_VERSION_STRING}
                                            SOVERSION ${KIDLETIME_SOVERSION})
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/kidletime_export.h
               kidletime.h
         DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

install(TARGETS KIdleTime EXPORT KIdleTimeTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
