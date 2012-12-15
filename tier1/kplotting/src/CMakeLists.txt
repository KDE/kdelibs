add_definitions(-DQT_NO_CAST_FROM_ASCII)

set(kplotting_SRCS
 kplotaxis.cpp
 kplotpoint.cpp
 kplotobject.cpp
 kplotwidget.cpp
)

add_library(kplotting ${kplotting_SRCS})
generate_export_header(kplotting)

target_link_libraries(kplotting ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

set_target_properties(kplotting PROPERTIES VERSION   ${KPLOTTING_VERSION_STRING}
                                           SOVERSION ${KPLOTTING_SOVERSION}
)

install(TARGETS kplotting EXPORT kplottingTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
  kplotaxis.h
  kplotpoint.h
  kplotobject.h
  kplotwidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/kplotting_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

