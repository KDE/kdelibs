project(kmetadata_ui)

include_directories(
  ${kmetadata_BINARY_DIR}
  ${kmetadata_SOURCE_DIR}
  ${libkmetadata_SOURCE_DIR}
  ${nepomuk_backbone_SOURCE_DIR}
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${KNEP_INCLUDE_DIR}
)

install(FILES
  kratingwidget.h
  ktagcloudwidget.h
  kmetadatatagcloud.h
  kmetadatatagwidget.h
  ktagdisplaywidget.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmetadata
)

install(FILES
  pics/rating.png
  DESTINATION ${DATA_INSTALL_DIR}/kmetadata/pics
)
