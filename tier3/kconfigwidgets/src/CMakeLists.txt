set(kconfigwidgets_SRCS
  kcodecaction.cpp
  kcolorscheme.cpp
  kcolorschememanager.cpp
  kconfigdialog.cpp
  kconfigviewstatesaver.cpp
  kconfigdialogmanager.cpp
  kcmodule.cpp
  klanguagebutton.cpp
  kpastetextaction.cpp
  krecentfilesaction.cpp
  kstandardaction.cpp
  ktip.cpp
)

add_library(KF5ConfigWidgets ${kconfigwidgets_SRCS})
generate_export_header(KF5ConfigWidgets BASE_NAME KConfigWidgets)
add_library(KF5::ConfigWidgets ALIAS KF5ConfigWidgets)

target_link_libraries(KF5ConfigWidgets
  PUBLIC
    KF5::Codecs # KCodecActions uses KCharsets, KEncodingProber
    KF5::WidgetsAddons # For K*Action, KPage*, KViewStateSerializer, KAcceleratorManager, K*GuiItem
    KF5::ConfigGui # KStandardAction uses KStandardShortcut
    KF5::Auth # KCModule uses KAuth::Action
  PRIVATE
    Qt5::DBus # KPasteTextAction talks to Klipper via DBus
    KF5::CoreAddons # KCModule uses KAboutData, KTip uses KRandom
    KF5::GuiAddons # KColorScheme uses KColorUtils
    KF5::I18n # For action and widget texts
)

set_target_properties(KF5ConfigWidgets PROPERTIES VERSION   ${KCONFIGWIDGETS_VERSION_STRING}
                                                  SOVERSION ${KCONFIGWIDGETS_SOVERSION}
                                                  EXPORT_NAME ConfigWidgets
)

install(TARGETS KF5ConfigWidgets EXPORT KF5ConfigWidgetsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  kcodecaction.h
  kcolorscheme.h
  kcolorschememanager.h
  kconfigdialog.h
  kconfigviewstatesaver.h
  kviewstatemaintainer.h
  kconfigdialogmanager.h
  kcmodule.h
  klanguagebutton.h
  kpastetextaction.h
  krecentfilesaction.h
  kstandardaction.h
  ktip.h
  ${CMAKE_CURRENT_BINARY_DIR}/kconfigwidgets_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

install( FILES
 ktip-bulb.png
 DESTINATION  ${DATA_INSTALL_DIR}/kconfigwidgets/pics
)

install(PROGRAMS preparetips DESTINATION ${BIN_INSTALL_DIR} )

