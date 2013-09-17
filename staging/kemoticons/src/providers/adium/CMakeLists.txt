
########### next target ###############
set(adiumemoticons_SRCS adium_emoticons.cpp)

kde4_add_plugin(emoticonstheme_adium ${adiumemoticons_SRCS})

target_link_libraries(emoticonstheme_adium
LINK_PRIVATE
    KEmoticons
    Qt5::Xml # for QDomDocument
)

########### install files ###############

install(TARGETS emoticonstheme_adium DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES emoticonstheme_adium.desktop DESTINATION  ${SERVICES_INSTALL_DIR})
