kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_create_doxygen_docs(  )

kde4_footer()



#original Makefile.am contents follow:

##
## this is a temporay solution to include bksys documentation in official kde documentation process 
##
#include ../admin/Doxyfile.am
