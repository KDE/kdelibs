
project(kutils)
include_directories( ${KDE4_KIO_INCLUDES} )
#add_subdirectory(ksettings/tests)

include_directories( ${CMAKE_SOURCE_DIR}/interfaces/kregexpeditor )

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

########### next target ###############

set(kutils_LIB_SRCS
kcmoduleinfo.cpp
kcmoduleloader.cpp
kcmultidialog.cpp
kcmoduleproxy.cpp
kpluginselector.cpp
kcmodulecontainer.cpp
ksettingswidgetadaptor.cpp
kcmodulefactory.cpp
${ksettings_STAT_SRCS}
)


kde4_add_library(kutils SHARED ${kutils_LIB_SRCS})

target_link_libraries(kutils  ${KDE4_KDECORE_LIBS} kdeui kio)

set_target_properties(kutils PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kutils  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES
 kutils_export.h
 kcmoduleinfo.h
 kcmoduleloader.h
 kcmultidialog.h
 kcmoduleproxy.h
 kpluginselector.h
 kcmodulecontainer.h
 DESTINATION ${INCLUDE_INSTALL_DIR})


