include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../.. ${KDE4_INCLUDES})

########### next target ###############
set(adiumemoticons_SRCS adium_emoticons.cpp)

kde4_add_plugin(emoticonstheme_adium ${adiumemoticons_SRCS})

target_link_libraries(emoticonstheme_adium kemoticons ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY})

########### install files ###############

install(TARGETS emoticonstheme_adium DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES emoticonstheme_adium.desktop DESTINATION  ${SERVICES_INSTALL_DIR})
