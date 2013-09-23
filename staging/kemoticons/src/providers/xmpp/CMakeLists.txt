
########### next target ###############
set(xmppemoticons_SRCS xmpp_emoticons.cpp)

add_library(emoticonstheme_xmpp MODULE ${xmppemoticons_SRCS})

target_link_libraries(emoticonstheme_xmpp
LINK_PRIVATE
    KEmoticons
    Qt5::Xml # for QDomDocument
)

########### install files ###############

install(TARGETS emoticonstheme_xmpp DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES emoticonstheme_xmpp.desktop DESTINATION  ${SERVICES_INSTALL_DIR})
