kde4_header()

project(kwallet)

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/client )

add_subdirectory( client ) 
add_subdirectory( backend ) 
add_subdirectory( tests ) 



########### install files ###############


kde4_create_doxygen_docs( REFERENCES dcop EXCLUDE backend  )

kde4_footer()



#original Makefile.am contents follow:

#
#SUBDIRS=client backend tests
#
#DOXYGEN_REFERENCES=dcop
#DOXYGEN_EXCLUDE=backend
#include ../admin/Doxyfile.am
#
