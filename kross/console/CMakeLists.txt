
set(kross_SRCS main.cpp )
kde4_add_executable(kross ${kross_SRCS})
target_link_libraries(kross krosscore ${KCoreAddons_LIBRARIES} KI18n kdeqt5staging Qt5::Widgets)
install(TARGETS kross ${INSTALL_TARGETS_DEFAULT_ARGS})
