set(kconfigwidgets_SRCS
  kconfigviewstatesaver.cpp
  kconfigdialogmanager.cpp
)

add_library(KConfigWidgets ${kconfigwidgets_SRCS})
generate_export_header(KConfigWidgets)

target_link_libraries(KConfigWidgets LINK_PUBLIC Qt5::Widgets ${KWidgetsAddons_LIBRARIES} ${KConfig_LIBRARIES} )

set_target_properties(KConfigWidgets PROPERTIES VERSION   ${KCONFIGWIDGETS_VERSION_STRING}
                                             SOVERSION ${KCONFIGWIDGETS_SOVERSION}
)

install(TARGETS KConfigWidgets EXPORT KConfigWidgetsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  kconfigviewstatesaver.h
  kviewstatemaintainer.h
  kconfigdialogmanager.h
  ${CMAKE_CURRENT_BINARY_DIR}/kconfigwidgets_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
