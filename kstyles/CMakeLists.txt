
include_directories( ${CMAKE_SOURCE_DIR}/kdefx ${KDE4_KDECORE_INCLUDES} )

add_subdirectory( keramik )
add_subdirectory( plastik )
add_subdirectory( utils )




#original Makefile.am contents follow:

#SUBDIRS = keramik plastik utils
## highcolor highcontrast themes light kthemestyle riscos
#
##kstyles should not be included in the apidocs
##include $(top_srcdir)/admin/Doxyfile.am
