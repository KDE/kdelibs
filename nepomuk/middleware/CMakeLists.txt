project(nepomukmiddleware)

include_directories( 
  ${nepomuk_SOURCE_DIR}
  ${nepomukmiddleware_SOURCE_DIR}/rdf
  ${nepomukmiddleware_SOURCE_DIR}/services
  ${KDE4_KDECORE_INCLUDES} 
  ${QT_INCLUDES}
  ${SOPRANO_INCLUDE_DIR} )

add_subdirectory(services)
add_subdirectory(rdf)

set(nepomuk_middleware_LIB_SRCS
  registry.cpp
  registrydbusbackend.cpp
  registrydbusinterface.cpp
  service.cpp
  servicedbusinterfacebackend.cpp
  message.cpp
  result.cpp
  dbustools.cpp
  servicepublisher.cpp
  servicepublisherdbusinterface.cpp
  servicewrapper.cpp
  services/dummyservice.cpp
  services/rdfrepository.cpp
  services/rdfrepositorypublisher.cpp
  services/rdfrepositorypublisherdbusinterface.cpp
  rdf/queryresulttable.cpp
  rdf/rdfdbus.cpp
  rdf/statementlistiterator.cpp
)

kde4_automoc(nepomuk-middleware ${nepomuk_middleware_LIB_SRCS})

kde4_add_library(nepomuk-middleware SHARED ${nepomuk_middleware_LIB_SRCS})

target_link_libraries(nepomuk-middleware
  ${SOPRANO_LIBRARIES}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
  ${QT_QTXML_LIBRARY}
  ${KDE4_KDECORE_LIBS})

set_target_properties(nepomuk-middleware PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS nepomuk-middleware DESTINATION ${LIB_INSTALL_DIR} )
install(FILES
  registry.h
  service.h
  servicedesc.h
  servicewrapper.h
  message.h
  result.h
  servicepublisher.h
  servicepublisherdbusinterface.h
  error.h
  dbustools.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/nepomuk
)
