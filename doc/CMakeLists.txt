kde4_header()

add_subdirectory( common ) 
add_subdirectory( kspell ) 
add_subdirectory( kdelibs ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#
#SUBDIRS = common kspell kdelibs
#
