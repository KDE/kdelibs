kde4_header()

find_package(ASPELL)
if (ASPELL_FOUND)
   add_subdirectory( aspell )
endif (ASPELL_FOUND)

 
find_package(HSPELL)
if (HSPELL_FOUND)
   add_subdirectory( hspell )
endif (HSPELL_FOUND)

add_subdirectory( ispell ) 

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
