project(libkmetadata)

include_directories(
  ${kmetadata_BINARY_DIR}
  ${kmetadata_SOURCE_DIR}
  ${libkmetadata_SOURCE_DIR}
  ${nepomuk_backbone_SOURCE_DIR}
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${KNEPOMUK_INCLUDE_DIR}
  ${nepomuk-kde_SOURCE_DIR}
)

set(kmetadata_LIB_SRCS
  variant.cpp
  resourcedata.cpp
  resourcemanager.cpp
  tools.cpp
  ui/kratingwidget.cpp
  ui/ktagcloudwidget.cpp
  ui/kblocklayout.cpp
  ui/kmetadatatagcloud.cpp
  ui/kmetadatatagwidget.cpp
  ui/kautoscrollarea.cpp
  ui/ktagdisplaywidget.cpp
  generated/resource.cpp
  generated/tag.cpp
)

kde4_automoc(${kmetadata_LIB_SRCS})

kde4_add_library(kmetadata SHARED ${kmetadata_LIB_SRCS})

add_dependencies(kmetadata rcgen)

target_link_libraries(kmetadata
  knepomuk
  konto
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
  ${KDE4_KDECORE_LIBRARY}
  ${KDE4_KDEUI_LIBRARY}
  ${KDE4_KDEFX_LIBRARY}
)

set_target_properties(kmetadata PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kmetadata DESTINATION ${LIB_INSTALL_DIR} )
install(FILES
  variant.h
  resourcemanager.h
  kmetadata.h
  tools.h
  kmetadata_export.h
  generated/resource.h
  generated/tag.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmetadata
)

add_subdirectory(ui)
