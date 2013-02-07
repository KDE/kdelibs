# Copyright (c) 2006-2009 Alexander Neundorf, <neundorf@kde.org>
# Copyright (c) 2006, 2007, Laurent Montel, <montel@kde.org>
# Copyright (c) 2007 Matthias Kretz <kretz@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# this is basically a copy of the qt4_get_moc_flags() macros from FindQt4.cmake
# which is for internal use only, so we should not use it here:
macro (_KDE4_GET_MOC_FLAGS _moc_flags)
   set(${_moc_flags})
   get_directory_property(_inc_DIRS INCLUDE_DIRECTORIES)

   foreach(_current ${_inc_DIRS})
      set(${_moc_flags} ${${_moc_flags}} "-I${_current}")
   endforeach(_current ${_inc_DIRS})

   get_directory_property(_defines COMPILE_DEFINITIONS)
   foreach(_current ${_defines})
      set(${_moc_flags} ${${_moc_flags}} "-D${_current}")
   endforeach(_current ${_defines})

   if(WIN32)
      set(${_moc_flags} ${${_moc_flags}} -DWIN32)
   endif(WIN32)

   # if Qt is installed only as framework, add -F /library/Frameworks to the moc arguments
   # otherwise moc can't find the headers in the framework include dirs
   if(APPLE  AND  "${QT_QTCORE_INCLUDE_DIR}" MATCHES "/Library/Frameworks/")
      set(${_moc_INC_DIRS} ${${_moc_INC_DIRS}} "-F/Library/Frameworks")
   endif(APPLE  AND  "${QT_QTCORE_INCLUDE_DIR}" MATCHES "/Library/Frameworks/")

endmacro(_KDE4_GET_MOC_FLAGS)


#create the implementation files from the ui files and add them to the list of sources
#usage: KDE4_ADD_UI3_FILES(foo_SRCS ${ui_files})
macro (KDE4_ADD_UI3_FILES _sources )

   _kde4_get_moc_flags(_moc_INCS)

   foreach (_current_FILE ${ARGN})

      get_filename_component(_tmp_FILE ${_current_FILE} ABSOLUTE)
      get_filename_component(_basename ${_tmp_FILE} NAME_WE)
      set(_header ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h)
      set(_src ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp)
      set(_moc ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc.cpp)

      add_custom_command(OUTPUT ${_header}
         COMMAND ${CMAKE_COMMAND}
         -DKDE3_HEADER:BOOL=ON
         -DKDE_UIC_EXECUTABLE:FILEPATH=${QT_UIC3_EXECUTABLE}
         -DKDE_UIC_FILE:FILEPATH=${_tmp_FILE}
         -DKDE_UIC_H_FILE:FILEPATH=${_header}
         -DKDE_UIC_BASENAME:STRING=${_basename}
         -DKDE_UIC_PLUGIN_DIR:FILEPATH="."
         -P ${KDE4_MODULE_DIR}/kde4uic.cmake
         MAIN_DEPENDENCY ${_tmp_FILE}
      )

# we need to run uic3 and replace some things in the generated file
      # this is done by executing the cmake script kde4uic.cmake
      add_custom_command(OUTPUT ${_src}
         COMMAND ${CMAKE_COMMAND}
         ARGS
         -DKDE3_IMPL:BOOL=ON
         -DKDE_UIC_EXECUTABLE:FILEPATH=${QT_UIC3_EXECUTABLE}
         -DKDE_UIC_FILE:FILEPATH=${_tmp_FILE}
         -DKDE_UIC_CPP_FILE:FILEPATH=${_src}
         -DKDE_UIC_H_FILE:FILEPATH=${_header}
         -DKDE_UIC_BASENAME:STRING=${_basename}
         -DKDE_UIC_PLUGIN_DIR:FILEPATH="."
         -P ${KDE4_MODULE_DIR}/kde4uic.cmake
         MAIN_DEPENDENCY ${_header}
      )

      add_custom_command(OUTPUT ${_moc}
         COMMAND ${QT_MOC_EXECUTABLE}
         ARGS ${_moc_INCS} ${_header} -o ${_moc}
         MAIN_DEPENDENCY ${_header}
      )
      list(APPEND ${_sources} ${_src} ${_moc} )

   endforeach (_current_FILE)
endmacro (KDE4_ADD_UI3_FILES)
