project(nepomukquery)

include_directories(
  ${QT_INCLUDES}
  ${KDE4_KDECORE_INCLUDES}
  ${CMAKE_SOURCE_DIR}
  ${SOPRANO_INCLUDE_DIR}
  ${NEPOMUK_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../core
  ${CMAKE_CURRENT_SOURCE_DIR}/../types
)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions(-DDISABLE_NEPOMUK_LEGACY=1)

set(nepomukquery_SRC
  term.cpp
  negationterm.cpp
  orterm.cpp
  andterm.cpp
  comparisonterm.cpp
  simpleterm.cpp
  groupterm.cpp
  resourceterm.cpp
  literalterm.cpp
  resourcetypeterm.cpp
  optionalterm.cpp
  query.cpp
  query_p.cpp
  filequery.cpp
  queryparser.cpp
  dateparser.cpp
  result.cpp
  queryserviceclient.cpp
  dbusoperators.cpp
  ../core/dbusconnectionpool.cpp
  queryserializer.cpp
)

soprano_add_ontology(nepomukquery_SRC "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nie.trig" "NIE" "Nepomuk::Vocabulary" "trig")
soprano_add_ontology(nepomukquery_SRC "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nfo.trig" "NFO" "Nepomuk::Vocabulary" "trig")

set_source_files_properties(
  org.kde.nepomuk.QueryService.xml
  PROPERTIES INCLUDE "dbusoperators_p.h")
set_source_files_properties(
  org.kde.nepomuk.Query.xml
  PROPERTIES INCLUDE "result.h")

qt4_add_dbus_interface(nepomukquery_SRC
  org.kde.nepomuk.QueryService.xml
  queryserviceinterface)
qt4_add_dbus_interface(nepomukquery_SRC
  org.kde.nepomuk.Query.xml
  queryinterface)

kde4_add_library(nepomukquery SHARED ${nepomukquery_SRC})

set_target_properties(nepomukquery PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

target_link_libraries(nepomukquery
  ${QT_QTCORE_LIBRARY}
  ${SOPRANO_LIBRARIES}
  kdecore
  nepomuk
  )

install(TARGETS nepomukquery EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
  term.h
  negationterm.h
  orterm.h
  andterm.h
  comparisonterm.h
  simpleterm.h
  groupterm.h
  resourceterm.h
  literalterm.h
  resourcetypeterm.h
  optionalterm.h
  query.h
  filequery.h
  result.h
  queryserviceclient.h
  queryparser.h
  nepomukquery_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/nepomuk COMPONENT Devel
)
