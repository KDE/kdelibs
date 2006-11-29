project(kmetadata)

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/kmetadata
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
)

set(kmetadata_LIB_SRCS
  variant.cpp
  resource.cpp
  resourcedata.cpp
  resourcemanager.cpp
  ontology.cpp
)

kde4_automoc(kmetadata ${kmetadata_LIB_SRCS})

kde4_add_library(kmetadata SHARED ${kmetadata_LIB_SRCS})

target_link_libraries(kmetadata knepclient ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${KDE4_KDECORE_LIBRARY})

set_target_properties(kmetadata PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kmetadata DESTINATION ${LIB_INSTALL_DIR} )
install(FILES
  variant.h
  resource.h
  resourcemanager.h
  kmetadata.h
  ontology.h
  kmetadata_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kmetadata
)
