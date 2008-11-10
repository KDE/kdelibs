project(kutils)
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
add_subdirectory(kemoticons)

set(kemoticons_LIB_SRCS
    kemoticons/kemoticons.cpp
    kemoticons/kemoticonstheme.cpp
    kemoticons/kemoticonsprovider.cpp
)

install(FILES kemoticons/kemoticonsTheme.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

install(FILES 
    kemoticons/kemoticons.h 
    kemoticons/kemoticonstheme.h 
    kemoticons/kemoticonsprovider.h 
    DESTINATION ${INCLUDE_INSTALL_DIR}
)

########### next target ###############

set(kutils_LIB_SRCS
kcmoduleinfo.cpp
kcmoduleloader.cpp
kcmultidialog.cpp
kcmoduleproxy.cpp
kpluginselector.cpp
kcmodulecontainer.cpp
ksettingswidgetadaptor.cpp
kprintpreview.cpp
${ksettings_STAT_SRCS}
${kemoticons_LIB_SRCS}
)


kde4_add_library(kutils SHARED ${kutils_LIB_SRCS})

target_link_libraries(kutils  ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} )
target_link_libraries(kutils  LINK_INTERFACE_LIBRARIES kdecore kdeui ${QT_QTGUI_LIBRARY} )

set_target_properties(kutils PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(TARGETS kutils EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( FILES
 kutils_export.h
 kcmoduleinfo.h
 kcmoduleloader.h
 kcmultidialog.h
 kcmoduleproxy.h
 kpluginselector.h
 kcmodulecontainer.h
 kprintpreview.h
 DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)



