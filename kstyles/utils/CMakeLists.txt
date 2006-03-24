kde4_header()

project(kstyle-utils)

add_subdirectory( installtheme ) 

include_directories( ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS= installtheme
#
