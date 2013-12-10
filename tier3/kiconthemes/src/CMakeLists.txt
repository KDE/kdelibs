
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
add_library(KF5::KIconThemes ALIAS KIconThemes)

target_link_libraries(KIconThemes
  PUBLIC
    Qt5::Widgets
  PRIVATE
    Qt5::DBus
    Qt5::Svg
    KF5::I18n          # for i18n in KIconDialog
    KF5::WidgetsAddons # for KPixmapSequence family
    KF5::ItemViews      # for KListWidgetSearchLine
    KF5::KConfigWidgets # for KColorScheme
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
  kiconengine.h
  ${CMAKE_CURRENT_BINARY_DIR}/kiconthemes_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
