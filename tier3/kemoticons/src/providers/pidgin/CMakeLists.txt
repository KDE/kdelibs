
########### next target ###############
set(pidginemoticons_SRCS pidgin_emoticons.cpp)

add_library(emoticonstheme_pidgin MODULE ${pidginemoticons_SRCS})

target_link_libraries(emoticonstheme_pidgin PRIVATE KEmoticons)

########### install files ###############

install(TARGETS emoticonstheme_pidgin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES emoticonstheme_pidgin.desktop DESTINATION  ${SERVICES_INSTALL_DIR})
