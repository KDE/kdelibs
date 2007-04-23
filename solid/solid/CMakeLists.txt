add_subdirectory( ifaces )

########### next target ###############

set(solid_LIB_SRCS 
   managerbase.cpp 
   frontendobject.cpp 
   device.cpp 
   devicemanager.cpp 
   deviceinterface.cpp 
   genericinterface.cpp 
   processor.cpp 
   block.cpp 
   storage.cpp 
   cdrom.cpp 
   volume.cpp 
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
)

kde4_automoc(${solid_LIB_SRCS})

kde4_add_library(solid SHARED ${solid_LIB_SRCS})

target_link_libraries(solid ${KDE4_KDECORE_LIBS} solidifaces )

set_target_properties(solid PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS solid DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES  soliddevicemanager.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR}  )
install( FILES solid_export.h device.h devicemanager.h deviceinterface.h genericinterface.h processor.h block.h storage.h cdrom.h volume.h opticaldisc.h camera.h portablemediaplayer.h networkhw.h acadapter.h battery.h button.h display.h audiohw.h dvbhw.h predicate.h powermanagement.h networking.h DESTINATION ${INCLUDE_INSTALL_DIR}/solid)

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



