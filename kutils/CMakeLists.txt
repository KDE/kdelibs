project(kutils)
include_directories( ${KDE4_KDEUI_INCLUDES} ${KDE4_KPARTS_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR})

########### kemoticons ################
add_subdirectory(kemoticons)

set(kemoticons_LIB_SRCS
    kemoticons/kemoticons.cpp
    kemoticons/kemoticonstheme.cpp
    kemoticons/kemoticonsprovider.cpp
)

kde4_add_library(kemoticons SHARED ${kemoticons_LIB_SRCS})
target_link_libraries(kemoticons ${KDE4_KIO_LIBS})
target_link_libraries(kemoticons LINK_INTERFACE_LIBRARIES kdecore kdeui ${QT_QTGUI_LIBRARY})
set_target_properties(kemoticons PROPERTIES
                      VERSION ${GENERIC_LIB_VERSION}
                      SOVERSION ${GENERIC_LIB_SOVERSION})

install(FILES kemoticons/kemoticonsTheme.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

install(FILES
    kemoticons_export.h
    kemoticons/kemoticons.h
    kemoticons/kemoticonstheme.h
    kemoticons/kemoticonsprovider.h
    DESTINATION ${INCLUDE_INSTALL_DIR}
    COMPONENT Devel
)

install(TARGETS kemoticons EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})


######## kidletime ####################
configure_file(kidletime/config-kidletime.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kidletime.h )

set(kidletime_LIB_SRCS
    kidletime/kidletime.cpp
    kidletime/abstractsystempoller.cpp
    kidletime/widgetbasedpoller.cpp
)

if (Q_WS_X11)
    if (HAVE_XSCREENSAVER)
      set(kidletime_LIB_SRCS ${kidletime_LIB_SRCS}
          kidletime/xscreensaverbasedpoller.cpp)
      qt4_add_dbus_interface(kidletime_LIB_SRCS kidletime/org.freedesktop.ScreenSaver.xml screensaver_interface)
    endif (HAVE_XSCREENSAVER)
    if (HAVE_XSYNC)
      set(kidletime_LIB_SRCS ${kidletime_LIB_SRCS}
          kidletime/xsyncbasedpoller.cpp )
    endif (HAVE_XSYNC)
elseif (Q_WS_MAC)
    set(kidletime_LIB_SRCS ${kidletime_LIB_SRCS} kidletime/macpoller.cpp)
elseif (Q_WS_WIN)
    set(kidletime_LIB_SRCS ${kidletime_LIB_SRCS} kidletime/windowspoller.cpp)
endif (Q_WS_X11)

kde4_add_library(kidletime SHARED ${kidletime_LIB_SRCS})
target_link_libraries(kidletime LINK_INTERFACE_LIBRARIES kdecore kdeui ${QT_QTGUI_LIBRARY})
target_link_libraries(kidletime ${KDE4_KDEUI_LIBS})

if (Q_WS_X11)
   if (HAVE_XSCREENSAVER)
     target_link_libraries(kidletime ${X11_Xss_LIB})
   endif (HAVE_XSCREENSAVER)
   if (HAVE_XSYNC)
     target_link_libraries(kidletime ${X11_Xext_LIB})
   endif (HAVE_XSYNC)
   target_link_libraries(kidletime ${X11_LIBRARIES})
endif (Q_WS_X11)

set_target_properties(kidletime PROPERTIES
                      VERSION ${GENERIC_LIB_VERSION}
                      SOVERSION ${GENERIC_LIB_SOVERSION})
install( FILES
 kidletime_export.h
 kidletime/kidletime.h
 DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

install(TARGETS kidletime EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})


########### kcmutils ###############
set(kcmutils_LIB_SRCS
    kcmoduleinfo.cpp
    kcmoduleloader.cpp
    kcmultidialog.cpp
    kcmoduleproxy.cpp
    kpluginselector.cpp
    kcmodulecontainer.cpp
    ksettingswidgetadaptor.cpp
    ksettings/dispatcher.cpp
    ksettings/dialog.cpp
    ksettings/pluginpage.cpp
    ksettings/componentsdialog.cpp
)

kde4_add_library(kcmutils SHARED ${kcmutils_LIB_SRCS})

target_link_libraries(kcmutils ${KDE4_KDEUI_LIBS})
target_link_libraries(kcmutils LINK_INTERFACE_LIBRARIES kdecore kdeui ${QT_QTGUI_LIBRARY})
set_target_properties(kcmutils PROPERTIES
                      VERSION ${GENERIC_LIB_VERSION}
                      SOVERSION ${GENERIC_LIB_SOVERSION})

install( FILES
 kcmutils_export.h
 kcmoduleinfo.h
 kcmoduleloader.h
 kcmultidialog.h
 kcmoduleproxy.h
 kpluginselector.h
 kcmodulecontainer.h
 DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

install(FILES
        ksettings/dispatcher.h
        ksettings/dialog.h
        ksettings/pluginpage.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/ksettings
        COMPONENT Devel)

install(TARGETS kcmutils EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})


########### kprintutils ###############
set(kprintutils_LIB_SRCS
    kprintpreview.cpp
)

kde4_add_library(kprintutils SHARED ${kprintutils_LIB_SRCS})

target_link_libraries(kprintutils ${KDE4_KPARTS_LIBS})
target_link_libraries(kprintutils LINK_INTERFACE_LIBRARIES kdecore kdeui ${QT_QTGUI_LIBRARY})
set_target_properties(kprintutils PROPERTIES
                      VERSION ${GENERIC_LIB_VERSION}
                      SOVERSION ${GENERIC_LIB_SOVERSION})

install( FILES
 kprintutils_export.h
 kprintpreview.h
 DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

install(TARGETS kprintutils EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})


########### kutils ###############
# This is only for backwards compatibility, don't use in new projects
if(NOT KDE_PLATFORM_FEATURE_DISABLE_DEPRECATED)
set(kutils_LIB_SRCS
    dummy.cpp
)

kde4_add_library(kutils SHARED ${kutils_LIB_SRCS})

target_link_libraries(kutils kemoticons kidletime kcmutils kprintutils ${QT_QTCORE_LIBRARY})
target_link_libraries(kutils LINK_INTERFACE_LIBRARIES kdecore kdeui ${QT_QTGUI_LIBRARY} kemoticons kidletime kcmutils kprintutils)
set_target_properties(kutils PROPERTIES
                      VERSION ${GENERIC_LIB_VERSION}
                      SOVERSION ${GENERIC_LIB_SOVERSION})

install(TARGETS kutils EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
endif(NOT KDE_PLATFORM_FEATURE_DISABLE_DEPRECATED)

