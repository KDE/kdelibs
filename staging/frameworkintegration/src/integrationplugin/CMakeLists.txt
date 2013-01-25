
set(plugin_SRCS frameworkintegrationplugin.cpp)

add_library(frameworkintegrationplugin MODULE ${plugin_SRCS})
set_target_properties(frameworkintegrationplugin PROPERTIES PREFIX "") # remove lib prefix

target_link_libraries(frameworkintegrationplugin kdeui)

install(TARGETS frameworkintegrationplugin DESTINATION ${QT_PLUGIN_INSTALL_DIR}/kf5)
