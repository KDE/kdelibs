add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

set(kiconthemes_SRCS
  kiconeffect.cpp
  kiconengine.cpp
  kiconloader.cpp
  kicontheme.cpp
  kpixmapsequence.cpp
  kpixmapsequenceoverlaypainter.cpp
  kpixmapsequencewidget.cpp
)

add_library(KIconThemes ${kiconthemes_SRCS})
generate_export_header(KIconThemes)

target_link_libraries(KIconThemes LINK_PUBLIC Qt5::Widgets Qt5::DBus KConfigGui KConfigCore KCoreAddons kguiaddons ki18n)

set_target_properties(KIconThemes PROPERTIES VERSION   ${KICONTHEMES_VERSION_STRING}
                                             SOVERSION ${KICONTHEMES_SOVERSION}
)

install(TARGETS KIconThemes EXPORT KIconThemesTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  kiconeffect.h
  kiconloader.h
  kicontheme.h
  kpixmapsequence.h
  kpixmapsequenceoverlaypainter.h
  kpixmapsequencewidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/kiconthemes_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
