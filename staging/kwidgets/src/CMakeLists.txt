
find_package(X11)
if(X11_FOUND)
  macro_bool_to_01(X11_FOUND HAVE_X11)
endif(X11_FOUND)


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
  dialogs/ktip.cpp
  utils/kcolorscheme.cpp
  utils/kpassivepopupmessagehandler.cpp
  utils/kstandardguiitem.cpp
  utils/kstandardshortcut.cpp
  widgets/kdragwidgetdecorator.cpp
  widgets/klanguagebutton.cpp
  widgets/kurllabel.cpp # Not good enough quality. Needs to use QUrl instead of QString and should not inherit QLabel, but hold it as a member instead.
  utils/urlhandler.cpp
)

add_library(KWidgets ${LIBRARY_TYPE} ${kwidgets_LIB_SRCS})
generate_export_header(KWidgets)

set(kwidgets_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/actions
  ${CMAKE_CURRENT_SOURCE_DIR}/utils
  ${CMAKE_CURRENT_SOURCE_DIR}/dialogs
  ${CMAKE_CURRENT_SOURCE_DIR}/widgets
)
target_include_directories(KWidgets PUBLIC "$<BUILD_INTERFACE:${kwidgets_INCLUDES}>"
)

target_link_libraries(KWidgets
  ${X11_LIBRARIES} ${X11_Xcursor_LIB}
  Qt5::Widgets
  Qt5::DBus

  ${KConfig_CORE_LIBRARY}
  ${KConfig_GUI_LIBRARY}
  ${KCoreAddons_LIBRARIES}
  KGuiAddons
  ${KWidgetsAddons_LIBRARIES}
  KI18n
  KCodecs
)

if(X11_FOUND)
  find_package(Qt5X11Extras 5.2.0 REQUIRED NO_MODULE)
  target_link_libraries(KWidgets Qt5::X11Extras)
endif()

set_target_properties(KWidgets PROPERTIES VERSION ${KWIDGETS_VERSION_STRING}
                                        SOVERSION ${KWIDGETS_SOVERSION}
)

install(TARGETS KWidgets EXPORT KWidgetsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  actions/kcodecaction.h
  actions/kfontaction.h
  actions/kfontsizeaction.h
  actions/kpastetextaction.h
  actions/krecentfilesaction.h
  actions/kselectaction.h
  actions/kstandardaction.h
  dialogs/kmessagebox.h
  dialogs/kmessageboxdontaskagaininterface.h
  dialogs/ktip.h
  utils/kcolorscheme.h
  utils/kpassivepopupmessagehandler.h
  utils/kstandardguiitem.h
  utils/kstandardshortcut.h
  widgets/kdragwidgetdecorator.h
  widgets/klanguagebutton.h
  widgets/kurllabel.h
  ${CMAKE_CURRENT_BINARY_DIR}/kwidgets_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

install( FILES
 dialogs/ktip-bulb.png
 DESTINATION  ${DATA_INSTALL_DIR}/kwidgets/pics
)
