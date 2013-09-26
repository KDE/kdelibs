
set(kiconthemes_SRCS
  kiconbutton.cpp
  kicondialog.cpp
  kiconeffect.cpp
  kiconengine.cpp
  kiconloader.cpp
  kicontheme.cpp
)

add_library(KIconThemes ${kiconthemes_SRCS})
generate_export_header(KIconThemes)

target_link_libraries(KIconThemes
  LINK_PUBLIC
    Qt5::Widgets
  LINK_PRIVATE
    Qt5::DBus
    Qt5::Svg
    KI18n          # for i18n in KIconDialog
    KWidgetsAddons # for KPixmapSequence family
    ItemViews      # for KListWidgetSearchLine
    KConfigWidgets # for KColorScheme
)

set_target_properties(KIconThemes PROPERTIES VERSION   ${KICONTHEMES_VERSION_STRING}
                                             SOVERSION ${KICONTHEMES_SOVERSION}
)

install(TARGETS KIconThemes EXPORT KIconThemesTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  kiconbutton.h
  kicondialog.h
  kiconeffect.h
  kiconloader.h
  kicontheme.h
  ${CMAKE_CURRENT_BINARY_DIR}/kiconthemes_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
