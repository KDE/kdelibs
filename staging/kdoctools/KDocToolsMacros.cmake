# Copyright (c) 2006-2009 Alexander Neundorf, <neundorf@kde.org>
# Copyright (c) 2006, 2007, Laurent Montel, <montel@kde.org>
# Copyright (c) 2007 Matthias Kretz <kretz@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


macro (KDE4_CREATE_HANDBOOK _docbook)
   get_filename_component(_input ${_docbook} ABSOLUTE)
   set(_doc ${CMAKE_CURRENT_BINARY_DIR}/index.cache.bz2)

   #Bootstrap
   if (_kdeBootStrapping)
      set(_ssheet "${CMAKE_BINARY_DIR}/staging/kdoctools/customization/kde-chunk.xsl")
      set(_bootstrapOption "--srcdir=${CMAKE_BINARY_DIR}/staging/kdoctools/")
   else (_kdeBootStrapping)
      set(_ssheet "${KDE4_DATA_INSTALL_DIR}/ksgmltools2/customization/kde-chunk.xsl")
      set(_bootstrapOption)
   endif (_kdeBootStrapping)

   file(GLOB _docs *.docbook)

#   if (CMAKE_CROSSCOMPILING)
#      set(IMPORT_MEINPROC4_EXECUTABLE "${KDE_HOST_TOOLS_PATH}/ImportMeinProc4Executable.cmake" CACHE FILEPATH "Point it to the export file of meinproc4 from a native build")
#      include(${IMPORT_MEINPROC4_EXECUTABLE})
#      set(KDE4_MEINPROC_EXECUTABLE meinproc4)
#   endif (CMAKE_CROSSCOMPILING)

   add_custom_command(OUTPUT ${_doc}
      COMMAND ${KDE4_MEINPROC_EXECUTABLE} --check ${_bootstrapOption} --cache ${_doc} ${_input}
      DEPENDS ${_docs} ${_KDE4_MEINPROC_EXECUTABLE_DEP} ${_ssheet}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
   )
   get_filename_component(_targ ${CMAKE_CURRENT_SOURCE_DIR} NAME)
   set(_targ "${_targ}-handbook")
   add_custom_target(${_targ} ALL DEPENDS ${_doc})

   if(KDE4_ENABLE_HTMLHANDBOOK)
      set(_htmlDoc ${CMAKE_CURRENT_SOURCE_DIR}/index.html)
      add_custom_command(OUTPUT ${_htmlDoc}
         COMMAND ${KDE4_MEINPROC_EXECUTABLE} --check ${_bootstrapOption} -o ${_htmlDoc} ${_input}
         DEPENDS ${_input} ${_KDE4_MEINPROC_EXECUTABLE_DEP} ${_ssheet}
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      )
      add_custom_target(htmlhandbook DEPENDS ${_htmlDoc})
   endif(KDE4_ENABLE_HTMLHANDBOOK)

   set(_args ${ARGN})

   set(_installDest)
   if(_args)
      list(GET _args 0 _tmp)
      if("${_tmp}" STREQUAL "INSTALL_DESTINATION")
         list(GET _args 1 _installDest )
         list(REMOVE_AT _args 0 1)
      endif("${_tmp}" STREQUAL "INSTALL_DESTINATION")
   endif(_args)

   get_filename_component(dirname ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)
   if(_args)
      list(GET _args 0 _tmp)
      if("${_tmp}" STREQUAL "SUBDIR")
         list(GET _args 1 dirname )
         list(REMOVE_AT _args 0 1)
      endif("${_tmp}" STREQUAL "SUBDIR")
   endif(_args)

   if(_installDest)
      file(GLOB _images *.png)
      install(FILES ${_doc} ${_docs} ${_images} DESTINATION ${_installDest}/${dirname})
      # TODO symlinks on non-unix platforms
      if (UNIX)
         # execute some cmake code on make install which creates the symlink
         install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${_installDest}/common\"  \"\$ENV{DESTDIR}${_installDest}/${dirname}/common\" )" )
      endif (UNIX)
   endif(_installDest)

endmacro (KDE4_CREATE_HANDBOOK)


macro (KDE4_CREATE_MANPAGE _docbook _section)
   get_filename_component(_input ${_docbook} ABSOLUTE)
   get_filename_component(_base ${_input} NAME)

   string(REGEX REPLACE "\\.${_section}\\.docbook$" "" _base ${_base})

   set(_doc ${CMAKE_CURRENT_BINARY_DIR}/${_base}.${_section})
   # sometimes we have "man-" prepended
   string(REGEX REPLACE "/man-" "/" _outdoc ${_doc})

   #Bootstrap
   if (_kdeBootStrapping)
      set(_ssheet "${CMAKE_BINARY_DIR}/staging/kdoctools/customization/kde-include-man.xsl")
      set(_bootstrapOption "--srcdir=${CMAKE_BINARY_DIR}/staging/kdoctools/")
   else (_kdeBootStrapping)
      set(_ssheet "${KDE4_DATA_INSTALL_DIR}/ksgmltools2/customization/kde-include-man.xsl")
      set(_bootstrapOption)
   endif (_kdeBootStrapping)

#   if (CMAKE_CROSSCOMPILING)
#      set(IMPORT_MEINPROC4_EXECUTABLE "${KDE_HOST_TOOLS_PATH}/ImportMeinProc4Executable.cmake" CACHE FILEPATH "Point it to the export file of meinproc4 from a native build")
#      include(${IMPORT_MEINPROC4_EXECUTABLE})
#      set(KDE4_MEINPROC_EXECUTABLE meinproc4)
#   endif (CMAKE_CROSSCOMPILING)

   add_custom_command(OUTPUT ${_outdoc}
      COMMAND ${KDE4_MEINPROC_EXECUTABLE} --stylesheet ${_ssheet} --check ${_bootstrapOption} ${_input}
      DEPENDS ${_input} ${_KDE4_MEINPROC_EXECUTABLE_DEP} ${_ssheet}
   )
   get_filename_component(_targ ${CMAKE_CURRENT_SOURCE_DIR} NAME)
   set(_targ "${_targ}-manpage-${_base}")
   add_custom_target(${_targ} ALL DEPENDS "${_outdoc}")

   set(_args ${ARGN})

   set(_installDest)
   if(_args)
      list(GET _args 0 _tmp)
      if("${_tmp}" STREQUAL "INSTALL_DESTINATION")
         list(GET _args 1 _installDest )
         list(REMOVE_AT _args 0 1)
      endif("${_tmp}" STREQUAL "INSTALL_DESTINATION")
   endif(_args)

   get_filename_component(dirname ${CMAKE_CURRENT_SOURCE_DIR} NAME_WE)
   if(_args)
      list(GET _args 0 _tmp)
      if("${_tmp}" STREQUAL "SUBDIR")
         list(GET _args 1 dirname )
         list(REMOVE_AT _args 0 1)
      endif("${_tmp}" STREQUAL "SUBDIR")
   endif(_args)

   if(_installDest)
      install(FILES ${_outdoc} DESTINATION ${_installDest}/man${_section})
   endif(_installDest)
endmacro (KDE4_CREATE_MANPAGE)

macro (KDE4_CREATE_HTML_HANDBOOK _docbook)
   message(STATUS "KDE4_CREATE_HTML_HANDBOOK() is deprecated. Enable the option KDE4_ENABLE_HTMLHANDBOOK instead, this will give you targets htmlhandbook for creating the html help.")
endmacro (KDE4_CREATE_HTML_HANDBOOK)
