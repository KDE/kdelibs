kde4_header()

# you can find macro_optional_find_package() in kdelibs/cmake/modules/OptionalFindPackage.cmake
# it is the same as FIND_PACKAGE(<name>) but additionally creates an OPTION(WITH_<name>)
# so the checking for the software can be disabled via ccmake or -DWITH_<name>=OFF
macro_optional_find_package(ASPELL)
if (ASPELL_FOUND)
   add_subdirectory( aspell )
endif (ASPELL_FOUND)

macro_optional_find_package(HSPELL)
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
