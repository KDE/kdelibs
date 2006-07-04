
project(kutils)

include_directories( ${KDE4_KIO_INCLUDES} )

include_directories( ${CMAKE_SOURCE_DIR}/interfaces/kregexpeditor )

add_subdirectory( ksettings )


# kutils/ksettings/Makefile.am: ksettings

set(ksettings_STAT_SRCS
  ${CMAKE_SOURCE_DIR}/kutils/ksettings/dispatcher.cpp
  ${CMAKE_SOURCE_DIR}/kutils/ksettings/dialog.cpp
  ${CMAKE_SOURCE_DIR}/kutils/ksettings/pluginpage.cpp
  ${CMAKE_SOURCE_DIR}/kutils/ksettings/componentsdialog.cpp
)

########### next target ###############

set(kutils_LIB_SRCS
kcmoduleinfo.cpp
kcmoduleloader.cpp
kcmultidialog.cpp
kmultitabbar.cpp
kcmoduleproxy.cpp
kpluginselector.cpp
kcmodulecontainer.cpp
kcmoduleproxyIface.cpp
kcmoduleproxyIfaceImpl.cpp
${ksettings_STAT_SRCS}
)

kde4_automoc(${kutils_LIB_SRCS})

kde4_add_library(kutils SHARED ${kutils_LIB_SRCS})

target_link_libraries(kutils  ${KDE4_KDECORE_LIBS} kdeui kio)

set_target_properties(kutils PROPERTIES VERSION 1.2.0 SOVERSION 1 )
install(TARGETS kutils  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install_files( /include FILES
 kcmoduleinfo.h
 kcmoduleloader.h
 kcmultidialog.h
 kmultitabbar.h 
 kcmoduleproxy.h
 kpluginselector.h
 kcmodulecontainer.h
)


