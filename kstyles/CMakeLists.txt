kde4_header()

macro_use_lib_kdecore()
include_directories( ${CMAKE_SOURCE_DIR}/kdefx ${QT_INCLUDES} )

add_subdirectory( keramik )
add_subdirectory( plastik )
add_subdirectory( utils )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = keramik plastik utils
## highcolor highcontrast themes light kthemestyle riscos
#
##kstyles should not be included in the apidocs
##include $(top_srcdir)/admin/Doxyfile.am
