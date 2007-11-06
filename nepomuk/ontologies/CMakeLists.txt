configure_file(nrl.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/nrl.desktop)
configure_file(nao.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/nao.desktop)
configure_file(nfo.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/nfo.desktop)
configure_file(rdf.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/rdf.desktop)
configure_file(rdfs.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/rdfs.desktop)
configure_file(dcq.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/dcq.desktop)
configure_file(dctype.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/dctype.desktop)
configure_file(dces.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/dces.desktop)

install(FILES
  nrl.rdfs
  ${CMAKE_CURRENT_BINARY_DIR}/nrl.desktop
  nao.nrl
  ${CMAKE_CURRENT_BINARY_DIR}/nao.desktop
  nfo.rdfs
  ${CMAKE_CURRENT_BINARY_DIR}/nfo.desktop
  rdf.rdfs
  ${CMAKE_CURRENT_BINARY_DIR}/rdf.desktop
  rdfs.rdfs
  ${CMAKE_CURRENT_BINARY_DIR}/rdfs.desktop
  ${CMAKE_CURRENT_BINARY_DIR}/dces.desktop
  dces.rdf
  ${CMAKE_CURRENT_BINARY_DIR}/dcq.desktop
  dcq.rdf
  ${CMAKE_CURRENT_BINARY_DIR}/dctype.desktop
  dctype.rdf
  DESTINATION
  ${DATA_INSTALL_DIR}/nepomuk/ontologies
)
