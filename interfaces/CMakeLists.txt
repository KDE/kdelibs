kde4_header()

project(interfaces)

macro_use_lib_kparts()
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

add_subdirectory( kdocument ) 
add_subdirectory( ktexteditor ) 
add_subdirectory( kregexpeditor ) 
add_subdirectory( kmediaplayer ) 
add_subdirectory( kio ) 
add_subdirectory( terminal ) 
add_subdirectory( khexedit ) 
add_subdirectory( kimproxy ) 
add_subdirectory( kspeech ) 

########### install files ###############


kde4_create_doxygen_docs( RECURSIVE REFERENCES kdecore kdefx kdeui kparts dcop EXCLUDE kspeech ktexteditor  )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = kdocument ktexteditor kregexpeditor kmediaplayer kio terminal khexedit kimproxy kspeech
#
#DOXYGEN_REFERENCES = kdecore kdefx kdeui kparts dcop
#DOXYGEN_EXCLUDE = kspeech ktexteditor
#DOXYGEN_SET_RECURSIVE = YES
#include ../admin/Doxyfile.am
