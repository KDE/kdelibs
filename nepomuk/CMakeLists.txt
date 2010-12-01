project(nepomuk)

include(SopranoAddOntology)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=300000)
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

if(KDE_NO_DEPRECATED)
   add_definitions(-DDISABLE_NEPOMUK_LEGACY=1)
endif(KDE_NO_DEPRECATED)


include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/core
  ${CMAKE_CURRENT_SOURCE_DIR}/types
  ${CMAKE_CURRENT_SOURCE_DIR}/ui
  ${CMAKE_CURRENT_SOURCE_DIR}/query
  ${CMAKE_CURRENT_SOURCE_DIR}/utils
  ${CMAKE_SOURCE_DIR}
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
  core/dbusconnectionpool.cpp
  core/resource.cpp
  core/thing.cpp
  core/file.cpp
  core/tag.cpp
  core/nepomukservice.cpp
  core/graphwrapper.cpp
)

if(NOT KDE_NO_DEPRECATED)
  set(nepomuk_core_SRCS
    ${nepomuk_core_SRCS}
    core/tools.cpp
  )
endif(NOT KDE_NO_DEPRECATED)

set(nepomuk_ui_SRCS
  ui/ktagcloudwidget.cpp
  ui/kblocklayout.cpp
  ui/kmetadatatagcloud.cpp
  ui/ktagdisplaywidget.cpp
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
  types/entitymanager.cpp
)

if(NOT KDE_NO_DEPRECATED)
  set(nepomuk_types_SRCS
    ${nepomuk_types_SRCS}
    types/desktopontologyloader.cpp
    types/fileontologyloader.cpp
    types/global.cpp
    types/nepomukontologyloader.cpp
    types/ontologyloader.cpp
    types/ontologymanager.cpp
  )
endif(NOT KDE_NO_DEPRECATED)

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

soprano_add_ontology(nepomuk_LIB_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nie.trig" "NIE" "Nepomuk::Vocabulary" "trig" VISIBILITY "nepomuk")
soprano_add_ontology(nepomuk_LIB_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nfo.trig" "NFO" "Nepomuk::Vocabulary" "trig" VISIBILITY "nepomuk")
soprano_add_ontology(nepomuk_LIB_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/ncal.trig" "NCAL" "Nepomuk::Vocabulary" "trig" VISIBILITY "nepomuk")
soprano_add_ontology(nepomuk_LIB_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nco.trig" "NCO" "Nepomuk::Vocabulary" "trig" VISIBILITY "nepomuk")
soprano_add_ontology(nepomuk_LIB_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nmo.trig" "NMO" "Nepomuk::Vocabulary" "trig" VISIBILITY "nepomuk")
soprano_add_ontology(nepomuk_LIB_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nmm.trig" "NMM" "Nepomuk::Vocabulary" "trig" VISIBILITY "nepomuk")
soprano_add_ontology(nepomuk_LIB_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nexif.trig" "NEXIF" "Nepomuk::Vocabulary" "trig" VISIBILITY "nepomuk")
soprano_add_ontology(nepomuk_LIB_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/pimo/pimo.trig" "PIMO" "Nepomuk::Vocabulary" "trig" VISIBILITY "nepomuk")
soprano_add_ontology(nepomuk_LIB_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/pimo/tmo.trig" "TMO" "Nepomuk::Vocabulary" "trig" VISIBILITY "nepomuk")
soprano_add_ontology(nepomuk_LIB_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nepomuk/nuao.trig" "NUAO" "Nepomuk::Vocabulary" "trig" VISIBILITY "nepomuk")
soprano_add_ontology(nepomuk_LIB_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nepomuk/ndo.trig" "NDO" "Nepomuk::Vocabulary" "trig" VISIBILITY "nepomuk")

kde4_add_library(nepomuk ${LIBRARY_TYPE} ${nepomuk_LIB_SRCS})

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
  core/nepomukservice.h
  core/resource.h
  core/thing.h
  core/tag.h
  core/file.h
  types/entity.h
  types/class.h
  types/property.h
  types/ontology.h
  types/literal.h
  ui/ktagcloudwidget.h
  ui/kmetadatatagcloud.h
  ui/kmetadatatagwidget.h
  ui/ktagdisplaywidget.h
  ui/nepomukmassupdatejob.h
  ui/tagwidget.h
  # Vocabularies
  ${CMAKE_CURRENT_BINARY_DIR}/nie.h
  ${CMAKE_CURRENT_BINARY_DIR}/nfo.h
  ${CMAKE_CURRENT_BINARY_DIR}/ncal.h
  ${CMAKE_CURRENT_BINARY_DIR}/nco.h
  ${CMAKE_CURRENT_BINARY_DIR}/nmo.h
  ${CMAKE_CURRENT_BINARY_DIR}/nmm.h
  ${CMAKE_CURRENT_BINARY_DIR}/nexif.h
  ${CMAKE_CURRENT_BINARY_DIR}/pimo.h
  ${CMAKE_CURRENT_BINARY_DIR}/tmo.h
  ${CMAKE_CURRENT_BINARY_DIR}/nuao.h
  ${CMAKE_CURRENT_BINARY_DIR}/ndo.h

  DESTINATION ${INCLUDE_INSTALL_DIR}/nepomuk COMPONENT Devel
)

if(NOT KDE_NO_DEPRECATED)
  install(FILES
    core/tools.h
    types/desktopontologyloader.h
    types/fileontologyloader.h
    types/global.h
    types/nepomukontologyloader.h
    types/ontologyloader.h
    types/ontologymanager.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/nepomuk COMPONENT Devel
  )
endif(NOT KDE_NO_DEPRECATED)

add_subdirectory(rcgen)
add_subdirectory(query)
add_subdirectory(utils)
add_subdirectory(test)
