include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../.. ${KDE4_INCLUDES})

########### next target ###############
set(kdeemoticons_SRCS kde_emoticons.cpp)

kde4_add_plugin(emoticonstheme_kde ${kdeemoticons_SRCS})

target_link_libraries(emoticonstheme_kde kemoticons ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY})

########### install files ###############

install(TARGETS emoticonstheme_kde DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES emoticonstheme_kde.desktop DESTINATION  ${SERVICES_INSTALL_DIR})
