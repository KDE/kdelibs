
########### next target ###############
set(kdeemoticons_SRCS kde_emoticons.cpp)

add_library(emoticonstheme_kde MODULE ${kdeemoticons_SRCS})

find_package(Qt5Xml REQUIRED)
target_link_libraries(emoticonstheme_kde
LINK_PRIVATE
    KEmoticons
    Qt5::Xml # for QDomDocument
)
########### install files ###############

install(TARGETS emoticonstheme_kde DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES emoticonstheme_kde.desktop DESTINATION  ${SERVICES_INSTALL_DIR})
