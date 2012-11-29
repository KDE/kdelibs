
set(kwidgetsaddons_SRCS
  kled.cpp
  kcapacitybar.cpp
  kruler.cpp
  kselector.cpp
  kxyselector.cpp
  kseparator.cpp
)

add_library(kwidgetsaddons ${kwidgetsaddons_SRCS})
generate_export_header(kwidgetsaddons)

target_link_libraries(kwidgetsaddons ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

set_target_properties(kwidgetsaddons PROPERTIES VERSION   ${KWIDGETSADDONS_VERSION_STRING}
                                                SOVERSION ${KWIDGETSADDONS_SOVERSION}
)

install(TARGETS kwidgetsaddons EXPORT kwidgetsaddonsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  kled.h
  kcapacitybar.h
  kruler.h
  kselector.h
  kxyselector.h
  kseparator.h
  ${CMAKE_CURRENT_BINARY_DIR}/kwidgetsaddons_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
