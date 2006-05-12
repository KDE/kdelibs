

project(interfaces)

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





#original Makefile.am contents follow:

#SUBDIRS = kdocument ktexteditor kregexpeditor kmediaplayer kio terminal khexedit kimproxy kspeech
#
#DOXYGEN_REFERENCES = kdecore kdefx kdeui kparts dcop
#DOXYGEN_EXCLUDE = kspeech ktexteditor
#DOXYGEN_SET_RECURSIVE = YES
#include ../admin/Doxyfile.am
