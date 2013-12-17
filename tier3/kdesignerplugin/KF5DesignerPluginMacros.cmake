# Copyright (c) 2006-2009 Alexander Neundorf, <neundorf@kde.org>
# Copyright (c) 2006, 2007, Laurent Montel, <montel@kde.org>
# Copyright (c) 2007 Matthias Kretz <kretz@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# The following variable is defined:
#
#  KF5DESIGNERPLUGIN_GENERATOR_EXECUTABLE - the executable to generate a plugin for Qt Designer
#
# It also adds the following macros and functions
#  KF5DESIGNERPLUGIN_ADD_WIDGET_FILES (SRCS_VAR file1.widgets ... fileN.widgets)
#    Use this to add widget description files for the Qt Designer plugin generator.
#

#FIXME: is this necessary once KF5DesignerPlugin is installed?
set(_KF5DESIGNERPLUGIN_GENERATOR_EXECUTABLE_DEP kgendesignerplugin)

macro (KF5DESIGNERPLUGIN_ADD_WIDGET_FILES _sources)
   foreach (_current_FILE ${_sources})

      get_filename_component(_input ${_current_FILE} ABSOLUTE)
      get_filename_component(_basename ${_input} NAME_WE)
      set(_source ${CMAKE_CURRENT_BINARY_DIR}/${_basename}widgets.cpp)
      set(_moc ${CMAKE_CURRENT_BINARY_DIR}/${_basename}widgets.moc)

      # create source file from the .widgets file
      add_custom_command(OUTPUT ${_source}
        COMMAND ${KF5DESIGNERPLUGIN_GENERATOR_EXECUTABLE}
        ARGS -o ${_source} ${_input}
        MAIN_DEPENDENCY ${_input} DEPENDS ${_KF5DESIGNERPLUGIN_GENERATOR_EXECUTABLE_DEP})

      # create moc file
      qt5_generate_moc(${_source} ${_moc} )

      list(APPEND ${_sources} ${_source} ${_moc})

   endforeach (_current_FILE)

endmacro (KF5DESIGNERPLUGIN_ADD_WIDGET_FILES)

