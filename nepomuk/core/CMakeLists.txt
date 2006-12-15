project(kmetadata)

find_package(KNep REQUIRED)


execute_process(
  COMMAND ${RCGEN} --listheaders --prefix ${CMAKE_CURRENT_BINARY_DIR}/ ${CMAKE_SOURCE_DIR}/ontology.rdf
  OUTPUT_VARIABLE ALLRESOURCES_HEADERS
)

execute_process(
  COMMAND ${RCGEN} --listsources --prefix ${CMAKE_CURRENT_BINARY_DIR}/ ${CMAKE_SOURCE_DIR}/ontology.rdf
  OUTPUT_VARIABLE ALLRESOURCES_SOURCES
)

execute_process(
  COMMAND ${RCGEN} --listincludes ${CMAKE_SOURCE_DIR}/ontology.rdf
  OUTPUT_VARIABLE ALLRESOURCES_INCLUDES
)

configure_file(allresources.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/allresources.h)

execute_process(
  COMMAND ${CMAKE_BINARY_DIR}/rcgen/rcgen --writeall ${CMAKE_CURRENT_BINARY_DIR}/ ${CMAKE_SOURCE_DIR}/ontology.rdf
)

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/kmetadata
  ${CMAKE_BINARY_DIR}
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${KNEP_INCLUDE_DIR}
)

set(kmetadata_LIB_SRCS
  variant.cpp
  resource.cpp
  resourcedata.cpp
  resourcemanager.cpp
  ontology.cpp
  ${ALLRESOURCES_SOURCES}
)

kde4_automoc(${kmetadata_LIB_SRCS})

kde4_add_library(kmetadata SHARED ${kmetadata_LIB_SRCS})

add_dependencies(kmetadata rcgen)

target_link_libraries(kmetadata ${KNEP_LIBRARIES} ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${KDE4_KDECORE_LIBRARY})

set_target_properties(kmetadata PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kmetadata DESTINATION ${LIB_INSTALL_DIR} )
install(FILES
  variant.h
  resource.h
  resourcemanager.h
  kmetadata.h
  kmetadata_export.h
  ${CMAKE_CURRENT_BINARY_DIR}/allresources.h
  ${ALLRESOURCES_HEADERS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmetadata
)
