include_directories( ${KDE4_KIO_INCLUDES} .. )

######### kmimefileparsertest ########

set(kmimefileparsertest_SRCS kmimefileparsertest.cpp ../kmimefileparser.cpp ../../kdecore/services/kmimemagicrule.cpp)


kde4_add_unit_test(kmimefileparsertest ${kmimefileparsertest_SRCS})

target_link_libraries(kmimefileparsertest ${KDE4_KIO_LIBS} ${QT_QTTEST_LIBRARY})

########### kded_test ###############
#
## not (yet ?) ported to dbus, Alex
#
# set(kded_test_PART_SRCS test.cpp )
#
#
# kde4_add_plugin(kded_test ${kded_test_PART_SRCS})
#
# target_link_libraries(kded_test  ${KDE4_KDECORE_LIBS} )
#
# install(TARGETS kded_test  DESTINATION ${PLUGIN_INSTALL_DIR} )
#
# install( FILES test.desktop  DESTINATION  ${SERVICES_INSTALL_DIR}/kded )

