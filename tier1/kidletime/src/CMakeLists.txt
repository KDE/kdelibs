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

if (X11_XSync_FOUND)
   set (HAVE_XSYNC 1)
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
    endif (HAVE_XSCREENSAVER)
    if (HAVE_XSYNC)
      set(kidletime_LIB_SRCS ${kidletime_LIB_SRCS}
          xsyncbasedpoller.cpp )
    endif (HAVE_XSYNC)
elseif (Q_OS_MAC)
    set(kidletime_LIB_SRCS ${kidletime_LIB_SRCS} macpoller.cpp)
elseif (Q_OS_WIN)
    set(kidletime_LIB_SRCS ${kidletime_LIB_SRCS} windowspoller.cpp)
endif (X11_FOUND)

add_library(kidletime ${LIBRARY_TYPE} ${kidletime_LIB_SRCS})
generate_export_header(kidletime)
target_link_libraries(kidletime LINK_INTERFACE_LIBRARIES ${QT_QTGUI_LIBRARY})
target_link_libraries(kidletime ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

if (X11_FOUND)
   if (HAVE_XSCREENSAVER)
     target_link_libraries(kidletime ${X11_Xscreensaver_LIB} ${QT_QTDBUS_LIBRARY})
   endif (HAVE_XSCREENSAVER)
   if (HAVE_XSYNC)
     target_link_libraries(kidletime ${X11_Xext_LIB})
   endif (HAVE_XSYNC)
   target_link_libraries(kidletime ${X11_LIBRARIES})
endif (X11_FOUND)

set_target_properties(kidletime PROPERTIES
                      VERSION ${ECM_VERSION_STRING}
                      SOVERSION ${ECM_SOVERSION})
install( FILES
 ${CMAKE_CURRENT_BINARY_DIR}/kidletime_export.h
 kidletime.h
 DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

install(TARGETS kidletime EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

