
include_directories(
    ${KDE4_KDECORE_INCLUDES}
    ${CMAKE_SOURCE_DIR}/kdeui
    ${CMAKE_SOURCE_DIR}/kdeui/util
    ${CMAKE_SOURCE_DIR}/kdeui/windowmanagement
    ${CMAKE_SOURCE_DIR}/kdecore
)

########### next target ###############
set(kwalletdefaultplugin_SRCS kwalletdefaultplugin.cpp)

set(kwallet_xml org.kde.KWallet.xml)
install(FILES ${kwallet_xml} DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

qt4_add_dbus_interface( kwalletdefaultplugin_SRCS ${kwallet_xml} kwallet_interface )

kde4_add_plugin(kwalletdefaultplugin ${kwalletdefaultplugin_SRCS})

target_link_libraries(kwalletdefaultplugin 
    ${KDE4_KDECORE_LIBS}
    ${KDEUI_EXTRA_LIBS}
    kdeui
    kdecore)


install(TARGETS kwalletdefaultplugin DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES kwalletdefaultplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})
