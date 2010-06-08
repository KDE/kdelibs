project(kparts)

add_subdirectory( tests )

include_directories( ${KDE4_KIO_INCLUDES} ${kparts_BINARY_DIR} ${nepomuk_SOURCE_DIR} ${nepomuk_SOURCE_DIR}/core ${nepomuk_SOURCE_DIR}/types ${SOPRANO_INCLUDE_DIR})


########### next target ###############

set(kparts_LIB_SRCS
   part.cpp
   plugin.cpp
   partmanager.cpp
   mainwindow.cpp
   event.cpp
   browserextension.cpp
   factory.cpp
   historyprovider.cpp
   browserinterface.cpp
   browserrun.cpp
   browseropenorsavequestion.cpp
   statusbarextension.cpp
   scriptableextension.cpp
   )

if(HAVE_NEPOMUK_WITH_SDO_0_5)
  soprano_add_ontology(kparts_LIB_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nie.trig" "NIE" "Nepomuk::Vocabulary" "trig")
  soprano_add_ontology(kparts_LIB_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nfo.trig" "NFO" "Nepomuk::Vocabulary" "trig")
  soprano_add_ontology(kparts_LIB_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nepomuk/ndo.trig" "NDO" "Nepomuk::Vocabulary" "trig")
  soprano_add_ontology(kparts_LIB_SRCS "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nepomuk/nuao.trig" "NUAO" "Nepomuk::Vocabulary" "trig")
endif(HAVE_NEPOMUK_WITH_SDO_0_5)

kde4_add_library(kparts SHARED ${kparts_LIB_SRCS})

target_link_libraries(kparts  ${KDE4_KDECORE_LIBS} kdeui kio)
target_link_libraries(kparts  LINK_INTERFACE_LIBRARIES kio kdeui kdecore ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} )
if(HAVE_NEPOMUK_WITH_SDO_0_5)
  target_link_libraries(kparts  nepomuk)
  target_link_libraries(kparts  LINK_INTERFACE_LIBRARIES nepomuk )
endif(HAVE_NEPOMUK_WITH_SDO_0_5)

set_target_properties(kparts PROPERTIES VERSION ${GENERIC_LIB_VERSION} 
                                        SOVERSION ${GENERIC_LIB_SOVERSION} 
                     )


install(TARGETS kparts EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( FILES kpart.desktop krop.desktop krwp.desktop browserview.desktop   DESTINATION  ${SERVICETYPES_INSTALL_DIR} )
install( FILES kparts_export.h part.h plugin.h partmanager.h mainwindow.h event.h
         browserextension.h factory.h historyprovider.h browserinterface.h genericfactory.h
         componentfactory.h browserrun.h statusbarextension.h browseropenorsavequestion.h
         scriptableextension.h
         DESTINATION  ${INCLUDE_INSTALL_DIR}/kparts COMPONENT Devel )

