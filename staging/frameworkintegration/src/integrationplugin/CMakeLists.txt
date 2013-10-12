
set(plugin_SRCS frameworkintegrationplugin.cpp)

add_library(FrameworkIntegrationPlugin MODULE ${plugin_SRCS})
set_target_properties(FrameworkIntegrationPlugin PROPERTIES PREFIX "") # remove lib prefix

target_link_libraries(FrameworkIntegrationPlugin LINK_PRIVATE KF5::KWidgetsAddons KF5::KConfigCore)

install(TARGETS FrameworkIntegrationPlugin DESTINATION ${QT_PLUGIN_INSTALL_DIR}/kf5)
