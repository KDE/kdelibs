include_directories(${CMAKE_CURRENT_SOURCE_DIR})

########### next target ###############

set(kdeemoticons_SRCS kde_emoticons.cpp)

kde4_add_plugin(kde_emoticonstheme ${kdeemoticons_SRCS})

install(TARGETS kde_emoticonstheme DESTINATION ${PLUGIN_INSTALL_DIR})
