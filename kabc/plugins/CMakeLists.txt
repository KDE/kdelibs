kde4_header()

add_subdirectory( file ) 
add_subdirectory( dir ) 
add_subdirectory( net ) 
add_subdirectory( ldapkio ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = file dir net ldapkio
