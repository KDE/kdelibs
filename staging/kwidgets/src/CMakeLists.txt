
find_package(X11)
if(X11_FOUND)
  macro_bool_to_01(X11_FOUND HAVE_X11)
endif(X11_FOUND)


# QUrl
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)


configure_file(config-kwidgets.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kwidgets.h )

set(kwidgets_LIB_SRCS
  actions/kcodecaction.cpp
  actions/kfontaction.cpp
  actions/kfontsizeaction.cpp
  actions/kpastetextaction.cpp
  actions/krecentfilesaction.cpp
  actions/kselectaction.cpp
  actions/kstandardaction.cpp
  dialogs/kmessagebox.cpp
  utils/kglobalsettings.cpp
  utils/kstandardguiitem.cpp
  utils/kstandardshortcut.cpp
  widgets/kdragwidgetdecorator.cpp
  widgets/kurllabel.cpp # Not good enough quality. Needs to use QUrl instead of QString and should not inherit QLabel, but hold it as a member instead.
)

add_library(kwidgets ${LIBRARY_TYPE} ${kwidgets_LIB_SRCS})
generate_export_header(kwidgets)

set(kwidgets_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/actions
  ${CMAKE_CURRENT_SOURCE_DIR}/utils
  ${CMAKE_CURRENT_SOURCE_DIR}/dialogs
  ${CMAKE_CURRENT_SOURCE_DIR}/widgets
)
target_include_directories(kwidgets PUBLIC "$<BUILD_INTERFACE:${kwidgets_INCLUDES}>"
)

target_link_libraries(kwidgets
  ${X11_LIBRARIES} ${X11_Xcursor_LIB}
  Qt5::Widgets
  Qt5::Svg
  Qt5::DBus

  kdeqt5staging # QUrlPathInfo
  ${KConfig_CORE_LIBRARY}
  ${KConfig_GUI_LIBRARY}
  ${KCoreAddons_LIBRARIES}
  kguiaddons
  ${KWidgetsAddons_LIBRARIES}
  ${KAuth_LIBRARIES}
  ${KWindowSystem_LIBRARIES}
  kservice
  ki18n
  KCodecs
  KIconThemes
)

if(X11_FOUND)
  find_package(Qt5X11Extras 5.2.0 REQUIRED NO_MODULE)
  target_link_libraries(kwidgets Qt5::X11Extras)
endif()

set_target_properties(kwidgets PROPERTIES VERSION ${KWIDGETS_VERSION_STRING}
                                        SOVERSION ${KWIDGETS_SOVERSION}
)

install(TARGETS kwidgets EXPORT kwidgetsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  actions/kcodecaction.h
  actions/kfontaction.h
  actions/kfontsizeaction.h
  actions/kpastetextaction.h
  actions/krecentfilesaction.h
  actions/kselectaction.h
  actions/kstandardaction.h
  dialogs/kmessagebox.h
  utils/kglobalsettings.h
  utils/kstandardguiitem.h
  utils/kstandardshortcut.h
  widgets/kdragwidgetdecorator.h
  widgets/kurllabel.h
  ${CMAKE_CURRENT_BINARY_DIR}/kwidgets_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
