project(kdeui)

# QUrl
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

find_package(X11)
if(X11_FOUND)
  macro_bool_to_01(X11_FOUND HAVE_X11)
endif(X11_FOUND)

configure_file(config-kdeui.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdeui.h )

add_subdirectory( about )
add_subdirectory( tests )
add_subdirectory( sonnet/tests )

########### next target ###############
set(kdeui_LIB_SRCS
 actions/kactionconflictdetector.cpp
 kernel/kstyle.cpp
 util/kglobalsettings.cpp
 util/khelpclient.cpp
 util/kimagecache.cpp
 widgets/kdeuiwidgetsproxystyle.cpp
)


kde4_add_ui_files(kdeui_LIB_SRCS ${kdeui_UI} )


kde4_add_library(kdeui ${LIBRARY_TYPE} ${kdeui_LIB_SRCS})
generate_export_header(kdeui)

target_include_directories(kdeui PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/..)

set(kdeui_INCLUDES
 ${CMAKE_CURRENT_SOURCE_DIR}/actions
 ${CMAKE_CURRENT_SOURCE_DIR}/colors
 ${CMAKE_CURRENT_SOURCE_DIR}/dialogs
 ${CMAKE_CURRENT_SOURCE_DIR}/findreplace
 ${CMAKE_CURRENT_SOURCE_DIR}/kernel
 ${CMAKE_CURRENT_SOURCE_DIR}/notifications
 ${CMAKE_CURRENT_SOURCE_DIR}/shortcuts
 ${CMAKE_CURRENT_SOURCE_DIR}/util
 ${CMAKE_CURRENT_SOURCE_DIR}/widgets
 ${CMAKE_CURRENT_BINARY_DIR}/util
 )
target_include_directories(kdeui PUBLIC "$<BUILD_INTERFACE:${kdeui_INCLUDES}>")

find_package(Qt5PrintSupport 5.2.0 REQUIRED NO_MODULE)

target_link_libraries(kdeui LINK_PUBLIC
    ${ItemModels_LIBRARIES}
    ${Sonnet_LIBRARIES}
    ${KAuth_LIBRARIES}
    KGuiAddons
    KWidgets
    ${KWidgetsAddons_LIBRARIES}
    KI18n
    KTextWidgets
    XmlGui
    ItemViews
    KCompletion
    KNotifications
    KJobWidgets
    KIconThemes
    ${KDE4_KDECORE_LIBS}
    ${X11_LIBRARIES}
    Qt5::Widgets
    Qt5::Svg
    Qt5::PrintSupport
    Qt5::Xml
    ${KDEUI_EXTRA_LIBS}
    Qt5::Network
    ${KCoreAddons_LIBRARIES}
    ${KWindowSystem_LIBRARIES}
    ${kdeqt5staging_LIBRARIES}
    ${KConfig_GUI_LIBRARY}
    KCompletion
    Qt5::Concurrent
)

if(X11_FOUND)
  find_package(Qt5X11Extras 5.2.0 REQUIRED NO_MODULE)
  target_link_libraries(kdeui LINK_PUBLIC Qt5::X11Extras)
endif()

if(NOT WINCE)
target_link_libraries(kdeui  Qt5::Svg)
endif(NOT WINCE)

if(MINGW)
   target_link_libraries(kdeui gdi32 oleaut32)
endif(MINGW)

if(APPLE)
   target_link_libraries(kdeui ${CARBON_LIBRARY})
endif(APPLE)

if(X11_XTest_FOUND)
   target_link_libraries(kdeui ${X11_XTest_LIB})
endif(X11_XTest_FOUND)

if(X11_Xcursor_FOUND)
   target_link_libraries(kdeui ${X11_Xcursor_LIB})
endif(X11_Xcursor_FOUND)

if (X11_Xfixes_FOUND)
   target_link_libraries(kdeui ${X11_Xfixes_LIB})
endif(X11_Xfixes_FOUND)

if (X11_Xrender_FOUND)
   target_link_libraries(kdeui ${X11_Xrender_LIB})
endif(X11_Xrender_FOUND)

target_link_libraries(kdeui LINK_INTERFACE_LIBRARIES ${KConfig_GUI_LIBRARY} Qt5::Widgets)

if(NOT WINCE)
target_link_libraries(kdeui LINK_INTERFACE_LIBRARIES Qt5::Svg)
endif(NOT WINCE)

set_target_properties(kdeui PROPERTIES VERSION ${KDE_NON_GENERIC_LIB_VERSION}
                                       SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION}
                       )

install(TARGETS kdeui EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
########### install files ###############


install( FILES
 dialogs/aboutkde.png
 dialogs/thumb_frame.png
 DESTINATION  ${DATA_INSTALL_DIR}/kdeui/pics )

install( FILES
 ${CMAKE_CURRENT_BINARY_DIR}/kdeui_export.h
 kernel/kstyle.h
 util/kglobalsettings.h
 util/khelpclient.h
 util/kimagecache.h
 DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

install(PROGRAMS preparetips DESTINATION ${BIN_INSTALL_DIR} )

if (APPLE)
  install(PROGRAMS kernel/start-session-bus.sh DESTINATION ${BIN_INSTALL_DIR} )
endif (APPLE)

install( FILES
    KDEUIMacros.cmake
    kde4uic.cmake
    DESTINATION ${CMAKECONFIG_INSTALL_DIR} COMPONENT Devel )
