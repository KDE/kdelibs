kde4_header()

add_subdirectory( kde ) 
add_subdirectory( message ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_create_doxygen_docs( REFERENCES kio kdecore kdeui  )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = kde message
#
#DOXYGEN_REFERENCES = kio kdecore kdeui
#include ../admin/Doxyfile.am
#
