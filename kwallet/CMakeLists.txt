
project(kwallet)

include_directories( ${KDE4_KDECORE_INCLUDES} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/client )

add_subdirectory( client ) 
add_subdirectory( backend ) 
add_subdirectory( tests ) 






#original Makefile.am contents follow:

#
#SUBDIRS=client backend tests
#
#DOXYGEN_REFERENCES=dcop
#DOXYGEN_EXCLUDE=backend
#include ../admin/Doxyfile.am
#
