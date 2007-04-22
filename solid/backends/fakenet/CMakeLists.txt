
add_subdirectory( tests )

########### next target ###############

set(solid_fakenet_PART_SRCS
   fakenetwork.cpp 
   fakewirelessnetwork.cpp
   fakenetworkinterface.cpp
   fakenetworkmanager.cpp
)

kde4_automoc(${solid_fakenet_PART_SRCS})

kde4_add_library(solid_fakenet_static STATIC ${solid_fakenet_PART_SRCS} )
target_link_libraries(solid_fakenet_static solidexperimentalifaces ${QT_QTXML_LIBRARY} )

kde4_add_plugin(solid_fakenet module.cpp ${solid_fakenet_PART_SRCS})


target_link_libraries(solid_fakenet solidexperimentalifaces ${QT_QTXML_LIBRARY} )

install(TARGETS solid_fakenet DESTINATION ${PLUGIN_INSTALL_DIR}  )


########### install files ###############

install( FILES solid_fakenet.desktop DESTINATION ${SERVICES_INSTALL_DIR}/solidbackends  )
install( FILES fakenetworking.xml DESTINATION ${DATA_INSTALL_DIR}/solidfakenetbackend  )


