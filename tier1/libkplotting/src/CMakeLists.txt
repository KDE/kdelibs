set(kplotting_SRCS
 kplotaxis.cpp
 kplotpoint.cpp
 kplotobject.cpp
 kplotwidget.cpp
)

kf5_add_library(kplotting ${LIBRARY_TYPE} ${kplotting_SRCS})

target_link_libraries(kplotting ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

set_target_properties(kplotting PROPERTIES VERSION   ${KDE_NON_GENERIC_LIB_VERSION}
                                            SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION}
)

install(TARGETS kplotting EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
  kplotaxis.h
  kplotpoint.h
  kplotobject.h
  kplotwidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/kplotting_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

