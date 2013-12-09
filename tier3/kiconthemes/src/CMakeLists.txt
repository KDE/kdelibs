
set(kiconthemes_SRCS
  kiconbutton.cpp
  kicondialog.cpp
  kiconeffect.cpp
  kiconengine.cpp
  kiconloader.cpp
  kicontheme.cpp
)

add_library(KF5IconThemes ${kiconthemes_SRCS})
generate_export_header(KF5IconThemes BASE_NAME KIconThemes)
add_library(KF5::IconThemes ALIAS KF5IconThemes)

target_link_libraries(KF5IconThemes
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

set_target_properties(KF5IconThemes PROPERTIES VERSION   ${KICONTHEMES_VERSION_STRING}
                                               SOVERSION ${KICONTHEMES_SOVERSION}
                                               EXPORT_NAME IconThemes
)

install(TARGETS KF5IconThemes EXPORT KF5IconThemesTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

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
