set(kded_kdeprintd_PART_SRCS kdeprintd.cpp kprintprocess.cpp )

kde4_add_plugin(kded_kdeprintd ${kded_kdeprintd_PART_SRCS})

target_link_libraries(kded_kdeprintd  ${KDE4_KDECORE_LIBS} kio )

install(TARGETS kded_kdeprintd  DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kdeprintd.desktop  DESTINATION  ${SERVICES_INSTALL_DIR}/kded )
