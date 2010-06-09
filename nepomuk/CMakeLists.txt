project(nepomuk)

include(SopranoAddOntology)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=300000)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/core
  ${CMAKE_CURRENT_SOURCE_DIR}/types
  ${CMAKE_CURRENT_SOURCE_DIR}/ui
  ${CMAKE_CURRENT_SOURCE_DIR}/query
  ${kdecore_SOURCE_DIR}/utils
  ${CMAKE_SOURCE_DIR}
  ${KDE4_INCLUDES}
  ${KDE4_KIO_INCLUDES}
  ${QT_INCLUDES}
  ${SOPRANO_INCLUDE_DIR}
)

set(nepomuk_core_SRCS
  core/variant.cpp
  core/resourcedata.cpp
  core/resourcemanager.cpp
  core/resourcefiltermodel.cpp
  core/nepomukmainmodel.cpp
  core/tools.cpp
  core/dbusconnectionpool.cpp
  core/resource.cpp
  core/thing.cpp
  core/tag.cpp
  core/nepomukservice.cpp
  core/graphwrapper.cpp
)

set(nepomuk_ui_SRCS
  ui/kratingwidget.cpp
  ui/ktagcloudwidget.cpp
  ui/kblocklayout.cpp
  ui/kmetadatatagcloud.cpp
  ui/ktagdisplaywidget.cpp
  ui/kratingpainter.cpp
  ui/nepomukmassupdatejob.cpp
  ui/tagwidget.cpp
  ui/kedittagsdialog.cpp
  ui/tagcheckbox.cpp
)

set(nepomuk_types_SRCS
  types/entity.cpp
  types/ontology.cpp
  types/class.cpp
  types/property.cpp
  types/literal.cpp
  types/ontologyloader.cpp
  types/ontologymanager.cpp
  types/nepomukontologyloader.cpp
  types/fileontologyloader.cpp
  types/desktopontologyloader.cpp
  types/global.cpp
  types/entitymanager.cpp
)

set(nepomuk_query_SRCS
  query/term.cpp
  query/negationterm.cpp
  query/orterm.cpp
  query/andterm.cpp
  query/comparisonterm.cpp
  query/simpleterm.cpp
  query/groupterm.cpp
  query/resourceterm.cpp
  query/literalterm.cpp
  query/resourcetypeterm.cpp
  query/optionalterm.cpp
  query/query.cpp
  query/query_p.cpp
  query/filequery.cpp
  query/queryparser.cpp
  query/dateparser.cpp
  query/result.cpp
  query/queryserviceclient.cpp
  query/dbusoperators.cpp
)

set(nepomuk_LIB_SRCS
  ${nepomuk_core_SRCS}
  ${nepomuk_ui_SRCS}
  ${nepomuk_types_SRCS}
#  ${nepomuk_query_SRCS}
)

soprano_add_ontology(nepomuk_LIB_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nie.trig" "NIE" "Nepomuk::Vocabulary" "trig")
soprano_add_ontology(nepomuk_LIB_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nfo.trig" "NFO" "Nepomuk::Vocabulary" "trig")
soprano_add_ontology(nepomuk_LIB_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nco.trig" "NCO" "Nepomuk::Vocabulary" "trig")
soprano_add_ontology(nepomuk_LIB_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/pimo/pimo.trig" "PIMO" "Nepomuk::Vocabulary" "trig")
soprano_add_ontology(nepomuk_LIB_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nepomuk/nuao.trig" "NUAO" "Nepomuk::Vocabulary" "trig")

kde4_add_library(nepomuk SHARED ${nepomuk_LIB_SRCS})

target_link_libraries(nepomuk
  ${SOPRANO_LIBRARIES}
  ${SOPRANO_CLIENT_LIBRARIES}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTDBUS_LIBRARY}
  kdecore
  kdeui
)

target_link_libraries(nepomuk
  LINK_INTERFACE_LIBRARIES
  kdeui
  ${SOPRANO_LIBRARIES})


set_target_properties(nepomuk PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS nepomuk EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  nepomuk_export.h
  core/variant.h
  core/resourcemanager.h
  core/tools.h
  core/nepomukservice.h
  core/resource.h
  core/thing.h
  core/tag.h
  types/entity.h
  types/class.h
  types/property.h
  types/ontology.h
  types/literal.h
  types/ontologymanager.h
  types/ontologyloader.h
  types/nepomukontologyloader.h
  types/fileontologyloader.h
  types/desktopontologyloader.h
  types/global.h
  ui/kratingwidget.h
  ui/ktagcloudwidget.h
  ui/kmetadatatagcloud.h
  ui/kmetadatatagwidget.h
  ui/ktagdisplaywidget.h
  ui/kratingpainter.h
  ui/nepomukmassupdatejob.h
  ui/tagwidget.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/nepomuk COMPONENT Devel
)

add_subdirectory(rcgen)
add_subdirectory(query)
add_subdirectory(test)
