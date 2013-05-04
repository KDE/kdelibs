
set(kwidgetsaddons_SRCS
  kdualaction.cpp
  kguiitem.cpp
  kled.cpp
  kcapacitybar.cpp
  kruler.cpp
  kselector.cpp
  kxyselector.cpp
  kseparator.cpp
  ktitlewidget.cpp
  ktoggleaction.cpp
  ktogglefullscreenaction.cpp
  kviewstateserializer.cpp
  kviewstatemaintainerbase.cpp
)

add_library(KWidgetsAddons ${kwidgetsaddons_SRCS})
generate_export_header(KWidgetsAddons)

target_link_libraries(KWidgetsAddons Qt5::Widgets)

set_target_properties(KWidgetsAddons PROPERTIES VERSION   ${KWIDGETSADDONS_VERSION_STRING}
                                                SOVERSION ${KWIDGETSADDONS_SOVERSION}
)

install(TARGETS KWidgetsAddons EXPORT KWidgetsAddonsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  kdualaction.h
  kguiitem.h
  kled.h
  kcapacitybar.h
  kruler.h
  kselector.h
  ktitlewidget.h
  kxyselector.h
  kseparator.h
  ktoggleaction.h
  ktogglefullscreenaction.h
  kviewstateserializer.h
  kviewstatemaintainerbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/kwidgetsaddons_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
