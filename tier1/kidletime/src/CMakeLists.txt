include_directories( ${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}
)

######## kidletime ####################
find_package(X11)

if (X11_XTest_FOUND)
   set (HAVE_XTEST 1)
else ()
   set (HAVE_XTEST 0)
endif ()

if (X11_Xscreensaver_FOUND)
   set (HAVE_XSCREENSAVER 1)
else ()
   set (HAVE_XSCREENSAVER 0)
endif ()

find_package(XCB)
find_package(X11_XCB)
if (QT5_BUILD AND X11_XSync_FOUND AND XCB_XCB_FOUND AND X11_XCB_FOUND AND XCB_SYNC_FOUND)
   set (HAVE_XSYNC 1)
   include_directories( ${XCB_XCB_INCLUDE_DIR} ${XCB_SYNC_INCLUDE_DIR})
else ()
   set (HAVE_XSYNC 0)
endif ()

configure_file(config-kidletime.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kidletime.h )

set(kidletime_LIB_SRCS
    kidletime.cpp
    abstractsystempoller.cpp
    widgetbasedpoller.cpp
)

if (X11_FOUND)
   if (HAVE_XSCREENSAVER)
      set(kidletime_LIB_SRCS ${kidletime_LIB_SRCS}
          xscreensaverbasedpoller.cpp)
      qt4_add_dbus_interface(kidletime_LIB_SRCS org.freedesktop.ScreenSaver.xml screensaver_interface)
   endif ()
   if (HAVE_XSYNC)
      set(kidletime_LIB_SRCS ${kidletime_LIB_SRCS}
          xsyncbasedpoller.cpp )
   endif ()
elseif (Q_OS_MAC)
   set(kidletime_LIB_SRCS ${kidletime_LIB_SRCS} macpoller.cpp)
elseif (Q_OS_WIN)
   set(kidletime_LIB_SRCS ${kidletime_LIB_SRCS} windowspoller.cpp)
endif (X11_FOUND)

add_library(kidletime ${kidletime_LIB_SRCS})
generate_export_header(kidletime)
target_link_libraries(kidletime LINK_PUBLIC ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})

if (X11_FOUND)
   if (HAVE_XSCREENSAVER)
      target_link_libraries(kidletime ${X11_Xscreensaver_LIB} ${QT_QTDBUS_LIBRARY})
   endif ()
   if (HAVE_XSYNC)
      target_link_libraries(kidletime ${X11_Xext_LIB} ${XCB_XCB_LIBRARIES} ${X11_XCB_LIBRARIES} ${XCB_SYNC_LIBRARIES})
   endif ()
   target_link_libraries(kidletime ${X11_LIBRARIES} ${kdeqt5staging_LIBRARY})
endif ()

set_target_properties(kidletime PROPERTIES  VERSION ${ECM_VERSION_STRING}
                                            SOVERSION ${ECM_SOVERSION})
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/kidletime_export.h
               kidletime.h
         DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

install(TARGETS kidletime EXPORT kdelibsLibraryTargets ${ECM_TARGET_DEFAULT_ARGS})
