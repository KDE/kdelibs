project(nepomukcore)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=300000)

include_directories(
  ${nepomuk_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/nepomuk/
  ${nepomukcore_SOURCE_DIR}
  ${nepomukcore_SOURCE_DIR}/types
  ${kdecore_SOURCE_DIR}/utils
  ${CMAKE_SOURCE_DIR}
  ${KDE4_INCLUDES}
  ${KDE4_KIO_INCLUDES}
  ${QT_INCLUDES}
  ${SOPRANO_INCLUDE_DIR}
)

set(nepomuk_LIB_SRCS
  variant.cpp
  resourcedata.cpp
  resourcemanager.cpp
  resourcefiltermodel.cpp
  nepomukmainmodel.cpp
  tools.cpp
  dbusconnectionpool.cpp
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
  resource.cpp
  thing.cpp
  tag.cpp
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
  nepomukservice.cpp
  graphwrapper.cpp
)

soprano_add_ontology(nepomuk_LIB_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nie.trig" "NIE" "Nepomuk::Vocabulary" "trig")
soprano_add_ontology(nepomuk_LIB_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nfo.trig" "NFO" "Nepomuk::Vocabulary" "trig")
soprano_add_ontology(nepomuk_LIB_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nco.trig" "NCO" "Nepomuk::Vocabulary" "trig")
soprano_add_ontology(nepomuk_LIB_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/pimo/pimo.trig" "PIMO" "Nepomuk::Vocabulary" "trig")

kde4_add_library(nepomuk SHARED ${nepomuk_LIB_SRCS})

# why is this here ? I don#t see where nepomuk depends on nepomuk_rcgen. Alex
add_dependencies(nepomuk nepomuk-rcgen)

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
  variant.h
  resourcemanager.h
  tools.h
  nepomukservice.h
  resource.h
  thing.h
  tag.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/nepomuk COMPONENT Devel
)

add_subdirectory(ui)
add_subdirectory(ontology)
add_subdirectory(test)
