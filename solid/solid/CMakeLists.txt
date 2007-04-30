#add_subdirectory( ifaces )
#add_subdirectory( backends )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

########### next target ###############

set(solid_LIB_SRCS 
   managerbase.cpp 
   device.cpp 
   devicemanager.cpp 
   deviceinterface.cpp 
   genericinterface.cpp 
   processor.cpp 
   block.cpp 
   storagedrive.cpp 
   cdrom.cpp 
   storagevolume.cpp 
   opticaldisc.cpp 
   camera.cpp 
   portablemediaplayer.cpp 
   networkhw.cpp 
   acadapter.cpp 
   battery.cpp 
   button.cpp
   display.cpp 
   audiohw.cpp 
   dvbhw.cpp
   predicate.cpp 
   predicateparse.cpp 
   predicate_lexer.c 
   predicate_parser.c
   powermanagement.cpp
   networking.cpp

   ifaces/acadapter.cpp
   ifaces/audiohw.cpp
   ifaces/battery.cpp
   ifaces/block.cpp
   ifaces/button.cpp
   ifaces/camera.cpp
   ifaces/cdrom.cpp
   ifaces/device.cpp
   ifaces/deviceinterface.cpp
   ifaces/devicemanager.cpp
   ifaces/display.cpp
   ifaces/dvbhw.cpp
   ifaces/genericinterface.cpp
   ifaces/networkhw.cpp
   ifaces/opticaldisc.cpp
   ifaces/portablemediaplayer.cpp
   ifaces/processor.cpp
   ifaces/storagedrive.cpp
   ifaces/storagevolume.cpp

   backends/hal/halacadapter.cpp
   backends/hal/halaudiohw.cpp
   backends/hal/halbattery.cpp
   backends/hal/halblock.cpp
   backends/hal/halbutton.cpp
   backends/hal/halcamera.cpp
   backends/hal/halcdrom.cpp
   backends/hal/haldeviceinterface.cpp
   backends/hal/haldisplay.cpp
   backends/hal/haldvbhw.cpp
   backends/hal/halgenericinterface.cpp
   backends/hal/halcalljob.cpp
   backends/hal/haldevice.cpp
   backends/hal/halmanager.cpp
   backends/hal/halnetworkhw.cpp
   backends/hal/halopticaldisc.cpp
   backends/hal/halportablemediaplayer.cpp
   backends/hal/halprocessor.cpp
   backends/hal/halstorage.cpp
   backends/hal/halvolume.cpp

   backends/fakehw/fakeacadapter.cpp
   backends/fakehw/fakeaudiohw.cpp
   backends/fakehw/fakebattery.cpp
   backends/fakehw/fakeblock.cpp
   backends/fakehw/fakebutton.cpp
   backends/fakehw/fakecamera.cpp
   backends/fakehw/fakecdrom.cpp
   backends/fakehw/fakedevice.cpp
   backends/fakehw/fakedeviceinterface.cpp
   backends/fakehw/fakedisplay.cpp
   backends/fakehw/fakedvbhw.cpp
   backends/fakehw/fakegenericinterface.cpp
   backends/fakehw/fakejob.cpp
   backends/fakehw/fakemanager.cpp
   backends/fakehw/fakenetworkhw.cpp
   backends/fakehw/fakeopticaldisc.cpp
   backends/fakehw/fakeportablemediaplayer.cpp
   backends/fakehw/fakeprocessor.cpp
   backends/fakehw/fakestorage.cpp
   backends/fakehw/fakevolume.cpp
)

qt4_add_dbus_interfaces_no_namespace(solid_LIB_SRCS
   org.kde.Solid.PowerManagement.xml
   org.kde.Solid.PowerManagement.Inhibit.xml
)
qt4_add_dbus_interface(solid_LIB_SRCS org.kde.Solid.Networking.xml org_kde_solid_networking)
kde4_automoc(${solid_LIB_SRCS})

kde4_add_library(solid SHARED ${solid_LIB_SRCS})

target_link_libraries(solid ${KDE4_KDECORE_LIBS} ${QT_QTXML_LIBRARY})

set_target_properties(solid PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS solid DESTINATION ${LIB_INSTALL_DIR} )


########### static lib for tests  ###############

kde4_add_library(solid_static STATIC ${solid_LIB_SRCS})

target_link_libraries(solid_static ${KDE4_KDECORE_LIBS} ${QT_QTXML_LIBRARY})


########### install files ###############

install( FILES solid_export.h device.h devicenotifier.h deviceinterface.h genericinterface.h processor.h block.h storagedrive.h cdrom.h storagevolume.h opticaldisc.h camera.h portablemediaplayer.h networkhw.h acadapter.h battery.h button.h display.h audiohw.h dvbhw.h predicate.h powermanagement.h networking.h DESTINATION ${INCLUDE_INSTALL_DIR}/solid)

########### parser build ###############

option(SOLID_PREDICATE_PARSER_UPDATE  "Update Solid predicate parser")

if(SOLID_PREDICATE_PARSER_UPDATE)

set(lexer_FILE predicate_lexer)
set(parser_FILE predicate_parser)

add_custom_command(OUTPUT ${lexer_FILE}.c
                   COMMAND flex ARGS -o${lexer_FILE}.c ${lexer_FILE}.l
                   DEPENDS ${lexer_FILE}.l
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_command(OUTPUT ${parser_FILE}.c ${parser_FILE}.h
                   COMMAND bison ARGS -d -b ${parser_FILE} ${parser_FILE}.y
                   COMMAND mv ARGS ${parser_FILE}.tab.c ${parser_FILE}.c
                   COMMAND mv ARGS ${parser_FILE}.tab.h ${parser_FILE}.h
                   DEPENDS ${lexer_FILE}.c
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

endif(SOLID_PREDICATE_PARSER_UPDATE)



