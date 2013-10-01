add_executable(kross main.cpp)
target_link_libraries(kross KrossCore ${KCoreAddons_LIBRARIES} KI18n Qt5::Widgets)

install(TARGETS kross ${INSTALL_TARGETS_DEFAULT_ARGS})
