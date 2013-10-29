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

macro (KDEWIDGETS_ADD_WIDGET_FILES _sources)
   foreach (_current_FILE ${ARGN})

      get_filename_component(_input ${_current_FILE} ABSOLUTE)
      get_filename_component(_basename ${_input} NAME_WE)
      set(_source ${CMAKE_CURRENT_BINARY_DIR}/${_basename}widgets.cpp)
      set(_moc ${CMAKE_CURRENT_BINARY_DIR}/${_basename}widgets.moc)

      # create source file from the .widgets file
      add_custom_command(OUTPUT ${_source}
        COMMAND ${KDEWIDGETS_MAKEKDEWIDGETS_EXECUTABLE}
        ARGS -o ${_source} ${_input}
        MAIN_DEPENDENCY ${_input} DEPENDS ${_KDEWIDGETS_MAKEKDEWIDGETS_DEP})

      # create moc file
      qt5_generate_moc(${_source} ${_moc} )

      list(APPEND ${_sources} ${_source} ${_moc})

   endforeach (_current_FILE)

endmacro (KDEWIDGETS_ADD_WIDGET_FILES)

