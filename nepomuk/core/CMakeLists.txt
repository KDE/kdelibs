project(libkmetadata)

include(KMetaDataMacros)

KMETADATA_GENERATE_FROM_ONTOLOGY(
  ${kmetadata_SOURCE_DIR}/nao.nrl
  ${libkmetadata_BINARY_DIR}
  NAO_HEADERS
  NAO_SOURCES
  NAO_INCLUDES
  ${libkmetadata_SOURCE_DIR}/resource.h.in
  ${libkmetadata_SOURCE_DIR}/resource.cpp.in
)

KMETADATA_GENERATE_FROM_ONTOLOGY(
  ${kmetadata_SOURCE_DIR}/ontologies/nie.nrl
  ${libkmetadata_BINARY_DIR}
  NIE_HEADERS
  NIE_SOURCES
  NIE_INCLUDES
)

# uses NAO_INCLUDES to generate a file that allows including all generated resources
configure_file(allresources.h.cmake ${libkmetadata_BINARY_DIR}/allresources.h)

include_directories(
  ${kmetadata_BINARY_DIR}
  ${kmetadata_SOURCE_DIR}
  ${libkmetadata_SOURCE_DIR}
  ${nepomuk_backbone_SOURCE_DIR}
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${KNEPOMUK_INCLUDE_DIR}
)

set(kmetadata_LIB_SRCS
  variant.cpp
  datetime.cpp
  resourcedata.cpp
  resourcemanager.cpp
  ontology.cpp
  tools.cpp
  ${NAO_SOURCES}
  ${NIE_SOURCES}
)

kde4_automoc(${kmetadata_LIB_SRCS})

kde4_add_library(kmetadata SHARED ${kmetadata_LIB_SRCS})

add_dependencies(kmetadata rcgen)

target_link_libraries(kmetadata
  knepomuk
  ${QT_QTCORE_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
  ${KDE4_KDECORE_LIBRARY}
)

set_target_properties(kmetadata PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kmetadata DESTINATION ${LIB_INSTALL_DIR} )
install(FILES
  variant.h
  resourcemanager.h
  kmetadata.h
  tools.h
  kmetadata_export.h
  ${libkmetadata_BINARY_DIR}/allresources.h
  ${NAO_HEADERS}
  ${NIE_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmetadata
)
