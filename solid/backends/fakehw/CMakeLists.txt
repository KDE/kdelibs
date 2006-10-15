

add_subdirectory( tests )

########### next target ###############

set(solid_fakehw_PART_SRCS 
   fakemanager.cpp 
   fakedevice.cpp 
   fakejob.cpp 
   fakecapability.cpp 
   fakeblock.cpp 
   fakestorage.cpp 
   fakecdrom.cpp 
   fakevolume.cpp 
   fakeopticaldisc.cpp 
   fakecamera.cpp 
   fakeportablemediaplayer.cpp 
   fakenetworkhw.cpp 
   fakeprocessor.cpp 
   fakeacadapter.cpp 
   fakebattery.cpp 
   fakebutton.cpp 
   fakedisplay.cpp
   fakeaudiohw.cpp )

kde4_automoc(solid_fakehw ${solid_fakehw_PART_SRCS})

kde4_add_library(solid_fakehw_static STATIC ${solid_fakehw_PART_SRCS} )
target_link_libraries(solid_fakehw_static solidifaces ${QT_QTXML_LIBRARY} )

kde4_add_plugin(solid_fakehw module.cpp ${solid_fakehw_PART_SRCS})


target_link_libraries(solid_fakehw solidifaces ${QT_QTXML_LIBRARY} )

install(TARGETS solid_fakehw DESTINATION ${PLUGIN_INSTALL_DIR}  )


########### install files ###############

install( FILES solid_fakehw.desktop DESTINATION ${SERVICES_INSTALL_DIR}/solidbackends  )
install( FILES fakecomputer.xml DESTINATION ${DATA_INSTALL_DIR}/solidfakehwbackend  )


