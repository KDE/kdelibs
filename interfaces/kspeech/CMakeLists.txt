kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/dcop )


########### install files ###############

install_files( ${SERVICETYPES_INSTALL_DIR} FILES dcoptexttospeech.desktop )
install_files( /include FILES kspeech.h kspeechsink.h )

kde4_footer()

