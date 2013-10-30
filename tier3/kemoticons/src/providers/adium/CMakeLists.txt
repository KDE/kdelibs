
########### next target ###############
set(adiumemoticons_SRCS adium_emoticons.cpp)

add_library(emoticonstheme_adium MODULE ${adiumemoticons_SRCS})

find_package(Qt5Xml REQUIRED)
target_link_libraries(emoticonstheme_adium
PRIVATE
    KEmoticons
    Qt5::Xml # for QDomDocument
)

########### install files ###############

install(TARGETS emoticonstheme_adium DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES emoticonstheme_adium.desktop DESTINATION  ${SERVICES_INSTALL_DIR})
