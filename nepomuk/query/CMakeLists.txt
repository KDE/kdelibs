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
  ${CMAKE_CURRENT_BINARY_DIR}/..
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
  filequery.cpp
  queryparser.cpp
  dateparser.cpp
  result.cpp
  queryserviceclient.cpp
  dbusoperators.cpp
  ../core/dbusconnectionpool.cpp
  queryserializer.cpp
  standardqueries.cpp
)

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

kde4_add_library(nepomukquery ${LIBRARY_TYPE} ${nepomukquery_SRC})

set_target_properties(nepomukquery PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

target_link_libraries(nepomukquery
  LINK_PRIVATE
  ${QT_QTCORE_LIBRARY}
  ${SOPRANO_LIBRARIES}
  kdecore
  nepomuk
  )

install(TARGETS nepomukquery EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
  andterm.h
  comparisonterm.h
  filequery.h
  groupterm.h
  literalterm.h
  negationterm.h
  nepomukquery_export.h
  optionalterm.h
  orterm.h
  query.h
  queryparser.h
  queryserviceclient.h
  resourceterm.h
  resourcetypeterm.h
  result.h
  simpleterm.h
  standardqueries.h
  term.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/nepomuk COMPONENT Devel
)
