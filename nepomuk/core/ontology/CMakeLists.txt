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
  DESTINATION ${INCLUDE_INSTALL_DIR}/nepomuk
)

if(KDE4_BUILD_TESTS)
  add_subdirectory(test)
endif(KDE4_BUILD_TESTS)
