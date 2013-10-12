set(kconfigwidgets_SRCS
  kcodecaction.cpp
  kcolorscheme.cpp
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

add_library(KConfigWidgets ${kconfigwidgets_SRCS})
generate_export_header(KConfigWidgets)
add_library(KF5::KConfigWidgets ALIAS KConfigWidgets)

target_link_libraries(KConfigWidgets
  LINK_PUBLIC
    KCodecs # KCodecActions uses KCharsets, KEncodingProber
    KF5::KWidgetsAddons # For K*Action, KPage*, KViewStateSerializer, KAcceleratorManager, K*GuiItem
    KF5::KConfigGui # KStandardAction uses KStandardShortcut
    KAuth # KCModule uses KAuth::Action
  LINK_PRIVATE
    Qt5::DBus # KPasteTextAction talks to Klipper via DBus
    KF5::KCoreAddons # KCModule uses KAboutData, KTip uses KRandom
    KGuiAddons # KColorScheme uses KColorUtils
    KF5::KI18n # For action and widget texts
)

set_target_properties(KConfigWidgets PROPERTIES VERSION   ${KCONFIGWIDGETS_VERSION_STRING}
                                             SOVERSION ${KCONFIGWIDGETS_SOVERSION}
)

install(TARGETS KConfigWidgets EXPORT KConfigWidgetsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  kcodecaction.h
  kcolorscheme.h
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

