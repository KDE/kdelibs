

add_subdirectory( ifaces )

include_directories( ${CMAKE_SOURCE_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### next target ###############

set(solid_LIB_SRCS 
   device.cpp 
   devicemanager.cpp 
   capability.cpp 
   processor.cpp 
   block.cpp 
   storage.cpp 
   cdrom.cpp 
   volume.cpp 
   opticaldisc.cpp 
   camera.cpp 
   portablemediaplayer.cpp 
   networkiface.cpp 
   acadapter.cpp 
   battery.cpp 
   button.cpp
   display.cpp 
   audioiface.cpp 
   predicate.cpp 
   predicateparse.cpp 
   predicate_lexer.c 
   predicate_parser.c
   powermanager.cpp
   networkdevice.cpp
   network.cpp
   wirelessnetwork.cpp
   networkmanager.cpp
   sysstatistics.cpp
)

kde4_automoc(${solid_LIB_SRCS})

kde4_add_library(solid SHARED ${solid_LIB_SRCS})

target_link_libraries(solid  ${KDE4_KIO_LIBS} solidifaces )

set_target_properties(solid PROPERTIES VERSION 4.2.0 SOVERSION 4 )
install(TARGETS solid DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES  soliddevicemanager.desktop solidpowermanager.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR}  )
install( FILES device.h devicemanager.h capability.h processor.h block.h storage.h cdrom.h volume.h opticaldisc.h camera.h portablemediaplayer.h networkiface.h acadapter.h battery.h button.h display.h audioiface.h predicate.h powermanager.h networkdevice.h network.h wirelessnetwork.h authentication.h sysstatistics.h DESTINATION ${INCLUDE_INSTALL_DIR}/solid)

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



