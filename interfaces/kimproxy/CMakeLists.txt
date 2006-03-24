kde4_header()

add_subdirectory( interface ) 
add_subdirectory( library ) 

include_directories( ${QT_INCLUDES}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = interface library