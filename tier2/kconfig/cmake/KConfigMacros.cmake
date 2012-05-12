# Copyright (c) 2006-2009 Alexander Neundorf, <neundorf@kde.org>
# Copyright (c) 2006, 2007, Laurent Montel, <montel@kde.org>
# Copyright (c) 2007 Matthias Kretz <kretz@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

macro (KCONFIG_ADD_KCFG_FILES _sources )
   foreach (_current_ARG ${ARGN})
       if( ${_current_ARG} STREQUAL "GENERATE_MOC" )
           set(_kcfg_generatemoc TRUE)
       endif( ${_current_ARG} STREQUAL "GENERATE_MOC" )

       if( ${_current_ARG} STREQUAL "USE_RELATIVE_PATH" )
           set(_kcfg_relativepath TRUE)
       endif( ${_current_ARG} STREQUAL "USE_RELATIVE_PATH" )
   endforeach (_current_ARG ${ARGN})

   foreach (_current_FILE ${ARGN})

     if(NOT ${_current_FILE} STREQUAL "GENERATE_MOC" AND NOT ${_current_FILE} STREQUAL "USE_RELATIVE_PATH")
       get_filename_component(_tmp_FILE ${_current_FILE} ABSOLUTE)
       get_filename_component(_abs_PATH ${_tmp_FILE} PATH)

       if (_kcfg_relativepath) # Process relative path only if the option was set
           # Get relative path
           get_filename_component(_rel_PATH ${_current_FILE} PATH)

           if (IS_ABSOLUTE ${_rel_PATH})
               # We got an absolute path
               set(_rel_PATH "")
           endif (IS_ABSOLUTE ${_rel_PATH})
       endif (_kcfg_relativepath)

       get_filename_component(_basename ${_tmp_FILE} NAME_WE)
       # If we had a relative path and we're asked to use it, then change the basename accordingly
       if(NOT ${_rel_PATH} STREQUAL "")
           set(_basename ${_rel_PATH}/${_basename})
       endif(NOT ${_rel_PATH} STREQUAL "")

       file(READ ${_tmp_FILE} _contents)
       string(REGEX REPLACE "^(.*\n)?File=([^\n]+kcfg).*\n.*$" "\\2"  _kcfg_FILENAME "${_contents}")
       set(_src_FILE    ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp)
       set(_header_FILE ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h)
       set(_moc_FILE    ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc)
       set(_kcfg_FILE   ${_abs_PATH}/${_kcfg_FILENAME})
       # Maybe the .kcfg is a generated file?
       if(NOT EXISTS "${_kcfg_FILE}")
           set(_kcfg_FILE   ${CMAKE_CURRENT_BINARY_DIR}/${_kcfg_FILENAME})
       endif(NOT EXISTS "${_kcfg_FILE}")
       if(NOT EXISTS "${_kcfg_FILE}")
           message(ERROR "${_kcfg_FILENAME} not found; tried in ${_abs_PATH} and ${CMAKE_CURRENT_BINARY_DIR}")
       endif(NOT EXISTS "${_kcfg_FILE}")

       # make sure the directory exist in the build directory
       if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${_rel_PATH}")
           file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${_rel_PATH})
       endif(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${_rel_PATH}")

#       if (CMAKE_CROSSCOMPILING)
#           set(IMPORT_KCONFIG_COMPILER_EXECUTABLE "${KDE_HOST_TOOLS_PATH}/ImportKConfigCompilerExecutable.cmake" CACHE FILEPATH "Point it to the export file of kconfig_compiler from a native build")
#           include(${IMPORT_KCONFIG_COMPILER_EXECUTABLE})
#           set(KDE4_KCFGC_EXECUTABLE kconfig_compiler)
#       endif (CMAKE_CROSSCOMPILING)

       # the command for creating the source file from the kcfg file
       add_custom_command(OUTPUT ${_header_FILE} ${_src_FILE}
          COMMAND ${KCONFIG_KCFGC_EXECUTABLE}
          ARGS ${_kcfg_FILE} ${_tmp_FILE} -d ${CMAKE_CURRENT_BINARY_DIR}/${_rel_PATH}
          MAIN_DEPENDENCY ${_tmp_FILE}
          DEPENDS ${_kcfg_FILE} ${_KDE4_KCONFIG_COMPILER_DEP} )

       if(_kcfg_generatemoc)
         qt4_generate_moc(${_header_FILE} ${_moc_FILE} )
         set_source_files_properties(${_src_FILE} PROPERTIES SKIP_AUTOMOC TRUE)  # don't run automoc on this file
         list(APPEND ${_sources} ${_moc_FILE})
       endif(_kcfg_generatemoc)

       list(APPEND ${_sources} ${_src_FILE} ${_header_FILE})
     endif(NOT ${_current_FILE} STREQUAL "GENERATE_MOC" AND NOT ${_current_FILE} STREQUAL "USE_RELATIVE_PATH")
   endforeach (_current_FILE)

endmacro (KCONFIG_ADD_KCFG_FILES)

