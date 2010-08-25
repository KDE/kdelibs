project(nepomuk_utils)

set(nepomukutils_SRCS
  global.cpp
)

soprano_add_ontology(nepomukutils_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nie.trig" "NIE" "Nepomuk::Vocabulary" "trig")
soprano_add_ontology(nepomukutils_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nfo.trig" "NFO" "Nepomuk::Vocabulary" "trig")
soprano_add_ontology(nepomukutils_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nepomuk/nuao.trig" "NUAO" "Nepomuk::Vocabulary" "trig")
soprano_add_ontology(nepomukutils_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nepomuk/ndo.trig" "NDO" "Nepomuk::Vocabulary" "trig")

kde4_add_library(nepomukutils ${LIBRARY_TYPE} ${nepomukutils_SRCS})

target_link_libraries(nepomukutils
  ${SOPRANO_LIBRARIES}
  ${QT_QTCORE_LIBRARY}
  nepomuk
  nepomukquery
  kdecore
)

set_target_properties(nepomukutils PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS nepomukutils EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  global.h
  nepomukutils_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/nepomuk COMPONENT Devel
)
