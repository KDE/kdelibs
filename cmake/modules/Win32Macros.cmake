# win32 macros
#
# ADDEXPLORERWRAPPER(project)
#
# Exists only under Win32 !
#
# addExplorerWrapper creates batch files for fast access 
# to the build environment from the win32 explorer. 
# 
# For mingw and nmake projects it opens a command shell,
# for Visual Studio IDE's (at least tested with VS 8 2005) it
# opens the related .sln file with paths setting specified at 
# configure time. 

# Copyright (c) 2006-2007, Ralf Habacker
#
# Redistribution and use is allowed according to the terms of the BSD license.


if (WIN32)
    #
    MACRO (addExplorerWrapper _projectname)
        # write explorer wrappers
        get_filename_component(CMAKE_BIN_PATH ${CMAKE_COMMAND} PATH)
        set (ADD_PATH "${CMAKE_BIN_PATH}")

        if (QT_QMAKE_EXECUTABLE)
            get_filename_component(QT_BIN_PATH ${QT_QMAKE_EXECUTABLE} PATH)
            set (ADD_PATH "${ADD_PATH};${QT_BIN_PATH}")
        endif (QT_QMAKE_EXECUTABLE)
        
        # add here more pathes 
        
        if (MINGW)
            get_filename_component(MINGW_BIN_PATH ${CMAKE_CXX_COMPILER} PATH)
            set (ADD_PATH "${ADD_PATH};${MINGW_BIN_PATH}")
            write_file (${CMAKE_BINARY_DIR}/${_projectname}-shell.bat "set PATH=${ADD_PATH};%PATH%\ncmd.exe")
        else (MINGW)
            if (CMAKE_BUILD_TOOL STREQUAL  "nmake")
                get_filename_component(VC_BIN_PATH ${CMAKE_CXX_COMPILER} PATH)
                write_file (${CMAKE_BINARY_DIR}/${_projectname}-shell.bat "set PATH=${ADD_PATH};%PATH%\ncall \"${VC_BIN_PATH}\\vcvars32.bat\"\ncmd.exe")
            else (CMAKE_BUILD_TOOL STREQUAL  "nmake")
                write_file (${CMAKE_BINARY_DIR}/${_projectname}-sln.bat "set PATH=${ADD_PATH};%PATH%\nstart ${_projectname}.sln")
            endif (CMAKE_BUILD_TOOL STREQUAL  "nmake")
        endif (MINGW)
    ENDMACRO (addExplorerWrapper)

    #
    # Set execution level for a target to 'asInvoker'
    #
    # Syntax:
    # set_execution_level_as_invoker(_source)
    #
    # @param _source name of source list for a given target
    #
    # Example:
    #   set (SOURCE test.c)
    #   set_execution_level_as_invoker(SOURCE)
    #   add_executable(atarget, ${SOURCE})
    #
    MACRO (set_execution_level_as_invoker _source)
        # set uiaccess to false in manifest
        file(READ ${CMAKE_SOURCE_DIR}/cmake/modules/Win32.Manifest.in _tmp)
        string(REPLACE "true" "false" _out ${_tmp})
        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${_source}.Win32.Manifest ${_out})
        # create rc file
        # 1 is the resource ID, ID_MANIFEST
        # 24 is the resource type, RT_MANIFEST
        # constants are used because of a bug in windres
        # see https://stackoverflow.com/questions/33000158/embed-manifest-file-to-require-administrator-execution-level-with-mingw32
        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${_source}.rc "1 24 \"${CMAKE_CURRENT_BINARY_DIR}/${_source}.Win32.Manifest\"\n")
        # add to source list
        list(APPEND ${_source} ${CMAKE_CURRENT_BINARY_DIR}/${_source}.rc)
    ENDMACRO (set_execution_level_as_invoker)
endif(WIN32)
