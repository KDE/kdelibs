
project(kcmkutils)
include_directories( ${KDE4_KDEUI_INCLUDES} ${KDE4_KPARTS_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR})

set(ksettings_STAT_SRCS
  ksettings/dispatcher.cpp
  ksettings/dialog.cpp
  ksettings/pluginpage.cpp
  ksettings/componentsdialog.cpp
)
install( FILES
 ksettings/dispatcher.h
 ksettings/dialog.h
 ksettings/pluginpage.h
DESTINATION  ${INCLUDE_INSTALL_DIR}/ksettings )

########### kemoticons ################
if(NOT ONLY_KLEO)
add_subdirectory(kemoticons)

set(kemoticons_LIB_SRCS
    kemoticons/kemoticons.cpp
    kemoticons/kemoticonstheme.cpp
    kemoticons/kemoticonsprovider.cpp
)

  kde4_add_library(kemoticons SHARED ${kemoticons_LIB_SRCS})
  
  target_link_libraries(kemoticons  ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} )
  
  set_target_properties(kemoticons PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install(FILES kemoticons/kemoticonsTheme.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

install(FILES 
    kemoticons_export.h
    kemoticons/kemoticons.h 
    kemoticons/kemoticonstheme.h 
    kemoticons/kemoticonsprovider.h 
    DESTINATION ${INCLUDE_INSTALL_DIR}
)

install( TARGETS kemoticons DESTINATION ${LIB_INSTALL_DIR} )

endif(NOT ONLY_KLEO)
  ########### next target ###############
  
set( kcmutils_LIB_SRCS
  kcmoduleinfo.cpp
  kcmoduleloader.cpp
  kcmultidialog.cpp
  kcmoduleproxy.cpp
  kcmodulecontainer.cpp
  ksettingswidgetadaptor.cpp
  kpluginselector.cpp
  ${ksettings_STAT_SRCS}
  )
  
kde4_add_library( kcmutils SHARED ${kcmutils_LIB_SRCS})
  
target_link_libraries( kcmutils ${KDE4_KDEUI_LIBS} )
set_target_properties( kcmutils PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
  
########### next target ###############
if(NOT ONLY_KLEO)

  project(kprintutils)
  
  set(kprintutils_LIB_SRCS
  kprintpreview.cpp
  )
  
  
  kde4_add_library(kprintutils SHARED ${kprintutils_LIB_SRCS})
  
  target_link_libraries(kprintutils  ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} )
  
  set_target_properties(kprintutils PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
endif(NOT ONLY_KLEO)
  
########### next target ###############
if(NOT ONLY_KLEO)
  # this is only for backwards compatibility, don't use in new projects
  project(kutils)
  kde4_add_library( kutils SHARED dummy.cpp )
  target_link_libraries( kutils kprintutils kcmutils kemoticons)
endif(NOT ONLY_KLEO)
 ########### install files ###############

install( TARGETS kcmutils DESTINATION ${LIB_INSTALL_DIR} )

 install( FILES
 kcmutils_export.h
  kcmoduleinfo.h
  kcmoduleloader.h
  kcmultidialog.h
  kcmoduleproxy.h
  kpluginselector.h
  kcmodulecontainer.h
  DESTINATION ${INCLUDE_INSTALL_DIR})

if (NOT ONLY_KLEO)
  install( TARGETS kprintutils kutils DESTINATION ${LIB_INSTALL_DIR} )

  install( FILES
   kprintutils_export.h
   kprintpreview.h
  DESTINATION ${INCLUDE_INSTALL_DIR})
endif(NOT ONLY_KLEO)
