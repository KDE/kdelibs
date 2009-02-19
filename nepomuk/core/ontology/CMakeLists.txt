project(nepomukcore_onto)

install(FILES
  entity.h
  class.h
  property.h
  ontology.h
  literal.h
  ontologymanager.h
  ontologyloader.h
  nepomukontologyloader.h
  fileontologyloader.h
  desktopontologyloader.h
  global.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/nepomuk COMPONENT Devel
)

add_subdirectory(test)
