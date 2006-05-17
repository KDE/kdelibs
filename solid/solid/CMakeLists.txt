kde4_header()

add_subdirectory( ifaces )

include_directories( ${CMAKE_SOURCE_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### next target ###############

set(kdehw_LIB_SRCS 
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
   predicate_parser.c )

kde4_automoc(${kdehw_LIB_SRCS})

kde4_add_library(kdehw SHARED ${kdehw_LIB_SRCS})

target_link_libraries(kdehw  ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} kdehwifaces )

set_target_properties(kdehw PROPERTIES VERSION 4.2.0 SOVERSION 4 )
install_targets(${LIB_INSTALL_DIR} kdehw )


########### install files ###############

install_files( ${SERVICETYPES_INSTALL_DIR} FILES  kdehwdevicemanager.desktop )
install_files( /include/kdehw FILES  device.h devicemanager.h capability.h processor.h block.h storage.h cdrom.h volume.h opticaldisc.h camera.h portablemediaplayer.h networkiface.h acadapter.h battery.h button.h display.h audioiface.h predicate.h )

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

kde4_footer()

