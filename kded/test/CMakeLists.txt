
# not (yet ?) ported to dbus, Alex

########### next target ###############

# set(kded_test_PART_SRCS test.cpp )
#
# kde4_automoc(${kded_test_PART_SRCS})
#
# set( kded_test_DCOP_SKEL_SRCS test.h )
#
# kde4_add_dcop_skels(kded_test_PART_SRCS ${kded_test_DCOP_SKEL_SRCS})
#
# kde4_add_plugin(kded_test ${kded_test_PART_SRCS})
#
# target_link_libraries(kded_test  ${KDE4_KDECORE_LIBS} )
#
# kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} kded_test )
install(TARGETS kded_test  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES test.desktop  DESTINATION  ${SERVICES_INSTALL_DIR}/kded )

