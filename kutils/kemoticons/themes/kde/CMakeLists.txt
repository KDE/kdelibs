include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../.. ${KDE4_INCLUDES})

########### next target ###############
set(kdeemoticons_SRCS kde_emoticons.cpp)

kde4_add_plugin(kde_emoticonstheme ${kdeemoticons_SRCS})

target_link_libraries(kde_emoticonstheme kemoticons ${KDE4_KDEUI_LIBS})

install(TARGETS kde_emoticonstheme DESTINATION ${PLUGIN_INSTALL_DIR})
