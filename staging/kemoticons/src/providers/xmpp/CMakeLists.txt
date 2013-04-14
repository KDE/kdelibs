
########### next target ###############
set(xmppemoticons_SRCS xmpp_emoticons.cpp)

kde4_add_plugin(emoticonstheme_xmpp ${xmppemoticons_SRCS})

target_link_libraries(emoticonstheme_xmpp kemoticons ${KDE4_KDEUI_LIBS} Qt5::Xml)

########### install files ###############

install(TARGETS emoticonstheme_xmpp DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES emoticonstheme_xmpp.desktop DESTINATION  ${SERVICES_INSTALL_DIR})
