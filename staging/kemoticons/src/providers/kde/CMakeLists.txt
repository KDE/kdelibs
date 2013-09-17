
########### next target ###############
set(kdeemoticons_SRCS kde_emoticons.cpp)

kde4_add_plugin(emoticonstheme_kde ${kdeemoticons_SRCS})

target_link_libraries(emoticonstheme_kde
LINK_PRIVATE
    KEmoticons
    Qt5::Xml # for QDomDocument
)
########### install files ###############

install(TARGETS emoticonstheme_kde DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES emoticonstheme_kde.desktop DESTINATION  ${SERVICES_INSTALL_DIR})
