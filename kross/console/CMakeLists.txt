
set(kross_SRCS main.cpp )
kde4_add_executable(kross ${kross_SRCS})
target_link_libraries(kross  ${KDE4_KDEUI_LIBS} krosscore ${KCoreAddons_LIBRARIES} )
install(TARGETS kross ${INSTALL_TARGETS_DEFAULT_ARGS})
