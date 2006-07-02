
project(kparts)


include_directories( ${KDE4_KIO_INCLUDES} )


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
   statusbarextension.cpp
   )

kde4_automoc(${kparts_LIB_SRCS})

kde4_add_library(kparts SHARED ${kparts_LIB_SRCS})

target_link_libraries(kparts  ${KDE4_KDECORE_LIBS} kdeui kio)

set_target_properties(kparts PROPERTIES VERSION 3.0.0 SOVERSION 3 )
install_targets(${LIB_INSTALL_DIR} kparts )


########### install files ###############

install_files( ${SERVICETYPES_INSTALL_DIR} FILES kpart.desktop krop.desktop krwp.desktop browserview.desktop  )
install_files( /include/kparts FILES part.h plugin.h partmanager.h mainwindow.h event.h   browserextension.h factory.h historyprovider.h browserinterface.h genericfactory.h     componentfactory.h browserrun.h statusbarextension.h )




