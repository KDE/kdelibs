
# Copyright (c) 2006-2009 Alexander Neundorf, <neundorf@kde.org>
# Copyright (c) 2006, 2007, Laurent Montel, <montel@kde.org>
# Copyright (c) 2007 Matthias Kretz <kretz@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


macro (KDE4_ADD_KCFG_FILES _sources )
   kconfig_add_kcfg_files( ${_sources} ${ARGN})
endmacro (KDE4_ADD_KCFG_FILES)


# TODO: This belongs whereever the rest of the kde translation system belongs
# Or in ECM, as it appears to be KDE independent, although it's odd to have such magic for TS files.
macro(KDE4_INSTALL_TS_FILES _lang _sdir)
   file(GLOB_RECURSE _ts_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${_sdir}/*)
   foreach(_current_TS_FILES ${_ts_files})
      string(REGEX MATCH "\\.svn/" _in_svn ${_current_TS_FILES})
      if(NOT _in_svn)
         get_filename_component(_subpath ${_current_TS_FILES} PATH)
         install(FILES ${_current_TS_FILES} DESTINATION ${LOCALE_INSTALL_DIR}/${_lang}/LC_SCRIPTS/${_subpath})
      endif(NOT _in_svn)
   endforeach(_current_TS_FILES)
endmacro(KDE4_INSTALL_TS_FILES)

