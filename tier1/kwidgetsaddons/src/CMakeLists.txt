
set(kwidgetsaddons_SRCS
  kled.cpp
  kcapacitybar.cpp
  kruler.cpp
  kselector.cpp
  kxyselector.cpp
  kseparator.cpp
)

add_library(kwidgetsaddons SHARED ${kwidgetsaddons_SRCS})
generate_export_header(kwidgetsaddons)

target_link_libraries(kwidgetsaddons ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

set_target_properties(kwidgetsaddons PROPERTIES VERSION   ${ECM_VERSION_STRING}
                                                SOVERSION ${ECM_SOVERSION}
)

install(TARGETS kwidgetsaddons EXPORT kdelibsLibraryTargets ${ECM_TARGET_DEFAULT_ARGS})

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
