kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/dcop ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( ${SERVICETYPES_INSTALL_DIR} FILES dcoptexttospeech.desktop )
install_files( /include FILES kspeech.h kspeechsink.h )

kde4_create_doxygen_docs( REFERENCES kdecore dcop kio kdeui kparts  )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES=-I$(top_srcdir)/dcop $(all_includes)
#
## Header files to be installed.
#include_HEADERS = kspeech.h kspeechsink.h
#
## Define DCOP/Text-to-Speech Service Type.
#kde_servicetypes_DATA = dcoptexttospeech.desktop
#
#DOXYGEN_REFERENCES = kdecore dcop kio kdeui kparts
#include ../../admin/Doxyfile.am
