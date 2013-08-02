set(kconfigwidgets_SRCS
  kcolorscheme.cpp
  kconfigdialog.cpp
  kconfigviewstatesaver.cpp
  kconfigdialogmanager.cpp
  kcmodule.cpp
  klanguagebutton.cpp
  krecentfilesaction.cpp
  ktip.cpp
)

add_library(KConfigWidgets ${kconfigwidgets_SRCS})
generate_export_header(KConfigWidgets)

target_link_libraries(KConfigWidgets LINK_PUBLIC
    Qt5::Widgets

    KWidgetsAddons
    KConfigCore
    KConfigGui
    KAuth
    KCoreAddons
    KI18n
)

target_link_libraries(KConfigWidgets LINK_PRIVATE
    KGuiAddons
)

set_target_properties(KConfigWidgets PROPERTIES VERSION   ${KCONFIGWIDGETS_VERSION_STRING}
                                             SOVERSION ${KCONFIGWIDGETS_SOVERSION}
)

install(TARGETS KConfigWidgets EXPORT KConfigWidgetsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  kcolorscheme.h
  kconfigdialog.h
  kconfigviewstatesaver.h
  kviewstatemaintainer.h
  kconfigdialogmanager.h
  kcmodule.h
  klanguagebutton.h
  krecentfilesaction.h
  ktip.h
  ${CMAKE_CURRENT_BINARY_DIR}/kconfigwidgets_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

install( FILES
 ktip-bulb.png
 DESTINATION  ${DATA_INSTALL_DIR}/kconfigwidgets/pics
)

