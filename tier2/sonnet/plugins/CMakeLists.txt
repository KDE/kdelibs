
include_directories(
 ${CMAKE_CURRENT_SOURCE_DIR}/../core ${KDE4_KDECORE_INCLUDES}
 )

# you can find macro_optional_find_package() in kdelibs/cmake/modules/OptionalFindPackage.cmake
# it is the same as FIND_PACKAGE(<name>) but additionally creates an OPTION(WITH_<name>)
# so the checking for the software can be disabled via ccmake or -DWITH_<name>=OFF
macro_optional_find_package(ASPELL)
macro_log_feature(ASPELL_FOUND "Aspell" "Spell checking support via Aspell" "http://aspell.net/" FALSE "" "This is not needed for spell checking if Enchant is provided or only Hebrew spell checking is required")
if (ASPELL_FOUND)
   add_subdirectory( aspell )
endif (ASPELL_FOUND)

macro_optional_find_package(HSPELL)
macro_log_feature(HSPELL_FOUND "HSpell" "Spell checking support for Hebrew" "http://ivrix.org.il/projects/spell-checker/" FALSE "" "Hebrew support can also be provided via Enchant, providing the correct Enchant backends are installed")
if (HSPELL_FOUND)
   add_subdirectory( hspell )
endif (HSPELL_FOUND)

macro_optional_find_package(ENCHANT)
macro_log_feature(ENCHANT_FOUND "Enchant" "Spell checking support via Enchant" "http://www.abisource.com/projects/enchant/" FALSE "" "")
if (ENCHANT_FOUND)
   add_subdirectory( enchant )
endif (ENCHANT_FOUND)

#macro_optional_find_package(HUNSPELL)
#macro_log_feature(HUNSPELL_FOUND "Hunspell" "Spell checking support via Hunspell" "http://hunspell.sourceforge.net/" FALSE "" "This is not needed for spell checking if Enchant or Aspell is provided or only Hebrew spell checking is required.")
#if (HUNSPELL_FOUND)
#   add_subdirectory( hunspell )
#endif (HUNSPELL_FOUND)

