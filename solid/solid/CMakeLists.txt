add_subdirectory( ifaces )
add_subdirectory( backends )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

if(WIN32)
   include_directories( ${KDEWIN32_INCLUDES} )
endif(WIN32) 

configure_file(solid_export.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/solid_export.h)

########### next target ###############

set(solid_LIB_SRCS 
   solidnamespace.cpp 
   managerbase.cpp 
   device.cpp 
   devicemanager.cpp 
   deviceinterface.cpp 
   genericinterface.cpp 
   processor.cpp 
   block.cpp 
   storagedrive.cpp 
   opticaldrive.cpp 
   storagevolume.cpp 
   opticaldisc.cpp 
   storageaccess.cpp 
   camera.cpp 
   portablemediaplayer.cpp 
   networkinterface.cpp 
   acadapter.cpp 
   battery.cpp 
   button.cpp
   audiointerface.cpp 
   dvbinterface.cpp
   predicate.cpp 
   predicateparse.cpp 
   predicate_lexer.c 
   predicate_parser.c
   powermanagement.cpp
   networking.cpp

   ifaces/acadapter.cpp
   ifaces/audiointerface.cpp
   ifaces/battery.cpp
   ifaces/block.cpp
   ifaces/button.cpp
   ifaces/camera.cpp
   ifaces/opticaldrive.cpp
   ifaces/device.cpp
   ifaces/deviceinterface.cpp
   ifaces/devicemanager.cpp
   ifaces/dvbinterface.cpp
   ifaces/genericinterface.cpp
   ifaces/networkinterface.cpp
   ifaces/opticaldisc.cpp
   ifaces/portablemediaplayer.cpp
   ifaces/processor.cpp
   ifaces/storagedrive.cpp
   ifaces/storagevolume.cpp
   ifaces/storageaccess.cpp

   backends/hal/halacadapter.cpp
   backends/hal/halaudiointerface.cpp
   backends/hal/halbattery.cpp
   backends/hal/halblock.cpp
   backends/hal/halbutton.cpp
   backends/hal/halcamera.cpp
   backends/hal/halcdrom.cpp
   backends/hal/haldeviceinterface.cpp
   backends/hal/haldvbinterface.cpp
   backends/hal/halgenericinterface.cpp
   backends/hal/haldevice.cpp
   backends/hal/halmanager.cpp
   backends/hal/halnetworkinterface.cpp
   backends/hal/halopticaldisc.cpp
   backends/hal/halportablemediaplayer.cpp
   backends/hal/halprocessor.cpp
   backends/hal/halstorageaccess.cpp
   backends/hal/halstorage.cpp
   backends/hal/halvolume.cpp

   backends/fakehw/fakeacadapter.cpp
   backends/fakehw/fakeaudiointerface.cpp
   backends/fakehw/fakebattery.cpp
   backends/fakehw/fakeblock.cpp
   backends/fakehw/fakebutton.cpp
   backends/fakehw/fakecamera.cpp
   backends/fakehw/fakecdrom.cpp
   backends/fakehw/fakedevice.cpp
   backends/fakehw/fakedeviceinterface.cpp
   backends/fakehw/fakedvbinterface.cpp
   backends/fakehw/fakegenericinterface.cpp
   backends/fakehw/fakemanager.cpp
   backends/fakehw/fakenetworkinterface.cpp
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

qt4_add_dbus_interface(solid_LIB_SRCS org.kde.Solid.Networking.Client.xml
    org_kde_solid_networking_client)


kde4_add_library(solid SHARED ${solid_LIB_SRCS})

target_link_libraries(solid ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTGUI_LIBRARY})

set_target_properties(solid PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS solid DESTINATION ${LIB_INSTALL_DIR} )

install(FILES org.kde.Solid.Networking.Client.xml DESTINATION ${DBUS_INTERFACES_DIR})

########### static lib for tests  ###############

kde4_add_library(solid_static STATIC ${solid_LIB_SRCS})

target_link_libraries(solid_static ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTGUI_LIBRARY})

if(WIN32)
  set_target_properties(solid_static PROPERTIES COMPILE_FLAGS -DSOLID_EXPORT=)
endif(WIN32)
########### install files ###############

install( FILES ${CMAKE_CURRENT_BINARY_DIR}/solid_export.h solidnamespace.h device.h devicenotifier.h deviceinterface.h genericinterface.h processor.h block.h storageaccess.h storagedrive.h opticaldrive.h storagevolume.h opticaldisc.h camera.h portablemediaplayer.h networkinterface.h acadapter.h battery.h button.h audiointerface.h dvbinterface.h predicate.h powermanagement.h networking.h DESTINATION ${INCLUDE_INSTALL_DIR}/solid)

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



