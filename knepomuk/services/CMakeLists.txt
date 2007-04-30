include_directories(
  ${nepomuk_backbone_SOURCE_DIR}
  ${QT_INCLUDES}
  ${KDE4_INCLUDES})

install(FILES
  dummyservice.h
  rdfrepository.h
  rdfrepositorypublisher.h
  resourceidservice.h
  resourceidservicepublisher.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/knepomuk/services
)
