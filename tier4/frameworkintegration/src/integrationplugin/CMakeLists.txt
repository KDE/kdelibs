
add_library(FrameworkIntegrationPlugin MODULE frameworkintegrationplugin.cpp)

target_link_libraries(FrameworkIntegrationPlugin PRIVATE KF5::WidgetsAddons KF5::ConfigCore KF5::Notifications)

install(TARGETS FrameworkIntegrationPlugin DESTINATION ${QT_PLUGIN_INSTALL_DIR}/kf5)
