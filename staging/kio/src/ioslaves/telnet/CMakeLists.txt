
add_executable(ktelnetservice ktelnetservice.cpp)
ecm_mark_nongui_executable(ktelnetservice)

target_link_libraries(ktelnetservice
   KF5::KCoreAddons
   KF5::KService
   KF5::KConfigGui
   KF5::KWidgetsAddons
   KF5::KI18n
   Qt5::Widgets
)

install(TARGETS ktelnetservice ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS ktelnetservice.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

