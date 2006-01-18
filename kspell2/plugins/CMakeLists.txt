kde4_header()

add_subdirectory( ispell ) 

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(LIBASPELLPLUGIN)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(LIBHSPELLPLUGIN)")
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#if include_aspell_plugin
#LIBASPELLPLUGIN = aspell
#endif
#
#if include_hspell_plugin
#LIBHSPELLPLUGIN = hspell
#endif
#
#SUBDIRS = $(LIBASPELLPLUGIN) $(LIBHSPELLPLUGIN) ispell 
