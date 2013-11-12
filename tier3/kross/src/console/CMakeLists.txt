#This binary is used for any non-GUI Kross scripts, e.g. Calligra Sheets has some
add_executable(kross main.cpp)
target_link_libraries(kross KrossCore KF5::KCoreAddons KF5::KI18n Qt5::Widgets)

install(TARGETS kross ${INSTALL_TARGETS_DEFAULT_ARGS})
