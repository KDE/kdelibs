
add_library(FrameworkIntegrationPlugin MODULE frameworkintegrationplugin.cpp)

target_link_libraries(FrameworkIntegrationPlugin PRIVATE KF5::KWidgetsAddons KF5::KConfigCore KF5::KNotifications)

install(TARGETS FrameworkIntegrationPlugin DESTINATION ${QT_PLUGIN_INSTALL_DIR}/kf5)
