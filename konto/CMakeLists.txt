project(konto)

include_directories( ${KDE4_KDECORE_INCLUDES} )
include_directories( ${QT_INCLUDES} ${SOPRANO_INCLUDE_DIR} )

set(konto_LIB_SRCS
  entity.cpp
  ontology.cpp
  class.cpp
  property.cpp
  literal.cpp
  ontologyloader.cpp
  qurlhash.cpp
  ontologymanager.cpp
  nepomukontologyloader.cpp
  fileontologyloader.cpp
  desktopontologyloader.cpp
  global.cpp
)

kde4_add_library(konto SHARED ${konto_LIB_SRCS})

target_link_libraries(
  konto
  knepomuk
  ${SOPRANO_LIBRARIES}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
  ${KDE4_KDECORE_LIBRARY}
)

set_target_properties(konto PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS konto DESTINATION ${LIB_INSTALL_DIR})
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
  konto_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/konto
)

if(KDE4_BUILD_TESTS)
  add_subdirectory(test)
endif(KDE4_BUILD_TESTS)
