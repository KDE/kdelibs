# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${KDE4_KIO_INCLUDES} .. )

######### kmimefileparsertest ########

set(kmimefileparsertest_SRCS kmimefileparsertest.cpp ../kmimefileparser.cpp ../../kdecore/services/kmimemagicrule.cpp)

kde4_automoc(${kmimefileparsertest_SRCS})

kde4_add_executable(kmimefileparsertest NOGUI RUN_UNINSTALLED ${kmimefileparsertest_SRCS})

target_link_libraries(kmimefileparsertest ${KDE4_KIO_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kded-kmimefileparsertest ${EXECUTABLE_OUTPUT_PATH}/kmimefileparsertest)



########### kded_test ###############
#
## not (yet ?) ported to dbus, Alex
#
# set(kded_test_PART_SRCS test.cpp )
#
# kde4_automoc(${kded_test_PART_SRCS})
#
# kde4_add_plugin(kded_test ${kded_test_PART_SRCS})
#
# target_link_libraries(kded_test  ${KDE4_KDECORE_LIBS} )
#
# install(TARGETS kded_test  DESTINATION ${PLUGIN_INSTALL_DIR} )
#
# install( FILES test.desktop  DESTINATION  ${SERVICES_INSTALL_DIR}/kded )

