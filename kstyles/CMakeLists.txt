kde4_header()

add_subdirectory( keramik ) 
add_subdirectory( plastik ) 
add_subdirectory( utils ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = keramik plastik utils 
## highcolor highcontrast themes light kthemestyle riscos 
#
##kstyles should not be included in the apidocs
##include $(top_srcdir)/admin/Doxyfile.am
