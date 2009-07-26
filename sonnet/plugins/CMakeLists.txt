
include_directories(
 ${CMAKE_SOURCE_DIR}/kdecore/sonnet ${KDE4_KDECORE_INCLUDES}
 )

# you can find macro_optional_find_package() in kdelibs/cmake/modules/OptionalFindPackage.cmake
# it is the same as FIND_PACKAGE(<name>) but additionally creates an OPTION(WITH_<name>)
# so the checking for the software can be disabled via ccmake or -DWITH_<name>=OFF
macro_optional_find_package(ASPELL)
macro_log_feature(ASPELL_FOUND "Aspell" "GNU Aspell spell checker" "http://aspell.net/" FALSE "" "Provides a spell checker for dozens of languages.")
if (ASPELL_FOUND)
   add_subdirectory( aspell )
endif (ASPELL_FOUND)

macro_optional_find_package(HSPELL)
macro_log_feature(HSPELL_FOUND "HSpell" "Hebrew spell checker" "http://ivrix.org.il/projects/spell-checker/" FALSE "" "Provides spell checking for Hebrew.")
if (HSPELL_FOUND)
   add_subdirectory( hspell )
endif (HSPELL_FOUND)

macro_optional_find_package(ENCHANT)
macro_log_feature(ENCHANT_FOUND "Enchant" "Enchant Package" "http://www.abisource.com/projects/enchant/" FALSE "" "Provides a way to access many spell checking backends.")
if (ENCHANT_FOUND)
   add_subdirectory( enchant )
endif (ENCHANT_FOUND)

#macro_optional_find_package(HUNSPELL)
#macro_log_feature(HUNSPELL_FOUND "Hunspell" "Hunspell Package" "http://hunspell.sourceforge.net/" FALSE "" "Provides a way to access many spell checking backends.")
#if (HUNSPELL_FOUND)
#   add_subdirectory( hunspell )
#endif (HUNSPELL_FOUND)

