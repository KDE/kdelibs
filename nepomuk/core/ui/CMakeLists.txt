project(nepomukcoreui)

include_directories(
  ${nepomuk_BINARY_DIR}
  ${nepomuk_SOURCE_DIR}
  ${nepomukcore_SOURCE_DIR}
  ${KDE4_INCLUDES} 
  ${QT_INCLUDES}
  ${SOPRANO_INCLUDE_DIR} )

install(FILES
  kratingwidget.h
  ktagcloudwidget.h
  kmetadatatagcloud.h
  kmetadatatagwidget.h
  ktagdisplaywidget.h
  kratingpainter.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/nepomuk COMPONENT Devel
)

install(FILES
  pics/rating.png
  DESTINATION ${DATA_INSTALL_DIR}/nepomuk/pics
)

add_subdirectory(test)
