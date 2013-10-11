add_definitions(-DQT_NO_CAST_FROM_ASCII)

set(kplotting_SRCS
 kplotaxis.cpp
 kplotpoint.cpp
 kplotobject.cpp
 kplotwidget.cpp
)

add_library(KPlotting ${kplotting_SRCS})
generate_export_header(KPlotting)
add_library(KF5::KPlotting ALIAS KPlotting)

target_link_libraries(KPlotting LINK_PUBLIC Qt5::Widgets)

set_target_properties(KPlotting PROPERTIES VERSION   ${KPLOTTING_VERSION_STRING}
                                           SOVERSION ${KPLOTTING_SOVERSION}
)

install(TARGETS KPlotting EXPORT KPlottingTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
  kplotaxis.h
  kplotpoint.h
  kplotobject.h
  kplotwidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/kplotting_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

