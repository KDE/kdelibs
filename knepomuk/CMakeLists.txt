project(knepomuk)

include_directories( ${KDE4_KDECORE_INCLUDES} )
include_directories( ${QT_INCLUDES} ${SOPRANO_INCLUDE_DIR} )

add_subdirectory(services)
add_subdirectory(rdf)

set(knepomuk_LIB_SRCS
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
  services/resourceidservice.cpp
  services/resourceidservicepublisher.cpp
  services/resourceidservicepublisherdbusinterface.cpp
  rdf/queryresulttable.cpp
  rdf/rdfdbus.cpp
  rdf/statementlistiterator.cpp
)

kde4_automoc(knepomuk ${knepomuk_LIB_SRCS})

kde4_add_library(knepomuk SHARED ${knepomuk_LIB_SRCS})

target_link_libraries(knepomuk
  ${SOPRANO_LIBRARIES}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
  ${QT_QTXML_LIBRARY}
  ${KDE4_KDECORE_LIBS})

set_target_properties(knepomuk PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS knepomuk DESTINATION ${LIB_INSTALL_DIR} )
install(FILES
  registry.h
  service.h
  servicedesc.h
  servicewrapper.h
  message.h
  result.h
  servicepublisher.h
  knepomuk.h
  knepomuk_export.h
  error.h
  dbustools.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/knepomuk
)
