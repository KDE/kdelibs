kde4_header()

add_subdirectory( ifaces ) 
add_subdirectory( ui ) 
add_subdirectory( tests ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(phononcore_LIB_SRCS 
   phononnamespace.cpp 
   base.cpp 
   factory.cpp 
   abstractmediaproducer.cpp 
   mediaobject.cpp 
   avcapture.cpp 
   audiopath.cpp 
   videopath.cpp 
   audiooutput.cpp 
   abstractvideooutput.cpp 
   audioeffect.cpp 
   videoeffect.cpp 
   abstractaudiooutput.cpp 
   audiodataoutput.cpp 
   simpleplayer.cpp 
   backendcapabilities.cpp 
   bytestream.cpp 
   namedescriptiontuple.cpp 
   audiosource.cpp 
   videosource.cpp 
   volumefadereffect.cpp )

kde4_automoc(${phononcore_LIB_SRCS})

kde4_add_dcop_skels(phononcore_LIB_SRCS factory.h mixeriface.h )

kde4_add_library(phononcore SHARED ${phononcore_LIB_SRCS})

target_link_libraries(phononcore  ${QT_AND_KDECORE_LIBS} kio phononifaces )

set_target_properties(phononcore PROPERTIES VERSION 0.0.0 SOVERSION 0 )
install_targets(${LIB_INSTALL_DIR} phononcore )


########### install files ###############

install_files( /include/phonon FILES  base.h phononnamespace.h abstractmediaproducer.h mediaobject.h avcapture.h mixeriface.h audiopath.h videopath.h audioeffect.h audiooutput.h videoeffect.h abstractvideooutput.h abstractaudiooutput.h audiodataoutput.h simpleplayer.h bytestream.h backendcapabilities.h namedescriptiontuple.h audiosource.h videosource.h volumefadereffect.h )
install_files( ${SERVICETYPES_INSTALL_DIR} FILES  phononbackend.desktop )

kde4_footer()
