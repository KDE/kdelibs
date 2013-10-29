# Copyright (c) 2006-2009 Alexander Neundorf, <neundorf@kde.org>
# Copyright (c) 2006, 2007, Laurent Montel, <montel@kde.org>
# Copyright (c) 2007 Matthias Kretz <kretz@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# The following variables are defined for the various tools required to
# compile KDE software:
#
#  KDEWIDGETS_MAKEKDEWIDGETS_EXECUTABLE - the makekdewidgets executable
#
# It also adds the following macros and functions
#  KDEWIDGETS_ADD_WIDGET_FILES (SRCS_VAR file1.widgets ... fileN.widgets)
#    Use this to add widget description files for the makekdewidgets code generator
#    for Qt Designer plugins.
#

if (WIN32)
    # CMAKE_CFG_INTDIR is the output subdirectory created e.g. by XCode and MSVC
    if (NOT WINCE)
        set(KDEWIDGETS_MAKEKDEWIDGETS_EXECUTABLE          ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/makekdewidgets )
    else (NOT WINCE)
        set(KDEWIDGETS_MAKEKDEWIDGETS_EXECUTABLE          ${HOST_BINDIR}/${CMAKE_CFG_INTDIR}/makekdewidgets )
    endif(NOT WINCE)

    set(KDEWIDGETS_MAKEKDEWIDGETS_EXECUTABLE          ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/makekdewidgets )
else (WIN32)
    set(KDEWIDGETS_MAKEKDEWIDGETS_EXECUTABLE          ${CMAKE_BINARY_DIR}/tier3/kdewidgets/src/makekdewidgets${CMAKE_EXECUTABLE_SUFFIX} )
endif (WIN32)

set( _KDEWIDGETS_MAKEKDEWIDGETS_EXECUTABLE_DEP makekdewidgets)

macro (KDEWIDGETS_ADD_WIDGET_FILES _sources)
   foreach (_current_FILE ${_sources})

      get_filename_component(_input ${_current_FILE} ABSOLUTE)
      get_filename_component(_basename ${_input} NAME_WE)
      set(_source ${CMAKE_CURRENT_BINARY_DIR}/${_basename}widgets.cpp)
      set(_moc ${CMAKE_CURRENT_BINARY_DIR}/${_basename}widgets.moc)

      # create source file from the .widgets file
      add_custom_command(OUTPUT ${_source}
        COMMAND ${KDEWIDGETS_MAKEKDEWIDGETS_EXECUTABLE}
        ARGS -o ${_source} ${_input}
        MAIN_DEPENDENCY ${_input} DEPENDS ${_KDEWIDGETS_MAKEKDEWIDGETS_EXECUTABLE_DEP})

      # create moc file
      qt5_generate_moc(${_source} ${_moc} )

      list(APPEND ${_sources} ${_source} ${_moc})

   endforeach (_current_FILE)

endmacro (KDEWIDGETS_ADD_WIDGET_FILES)

