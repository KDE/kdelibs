project(nepomukcoreui)

include_directories(
  ${nepomuk_BINARY_DIR}
  ${nepomuk_SOURCE_DIR}
  ${nepomukcore_SOURCE_DIR}
)

include_directories( ${KDE4_KDEUI_INCLUDES} )
include_directories( ${QT_INCLUDES} ${SOPRANO_INCLUDE_DIR} )

install(FILES
  kratingwidget.h
  ktagcloudwidget.h
  kmetadatatagcloud.h
  kmetadatatagwidget.h
  ktagdisplaywidget.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/nepomuk
)

install(FILES
  pics/rating.png
  DESTINATION ${DATA_INSTALL_DIR}/nepomuk/pics
)
