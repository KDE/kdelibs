set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

########### next target ###############

set(kpartstest_SRCS
testmainwindow.cpp
parts.cpp
notepad.cpp
)


kde4_add_executable(kpartstest TEST ${kpartstest_SRCS})

target_link_libraries(kpartstest  ${KDE4_KPARTS_LIBS} Qt5::Test ${KCoreAddons_LIBRARIES} )


########### next target ###############

set(normalktmtest_SRCS
normalktm.cpp
parts.cpp
notepad.cpp
)


kde4_add_executable(normalktmtest TEST ${normalktmtest_SRCS})

target_link_libraries(normalktmtest  ${KDE4_KPARTS_LIBS} Qt5::Test ${KCoreAddons_LIBRARIES} )


########### next target ###############

set(partviewer_SRCS
partviewer.cpp
)

kde4_add_executable(partviewer TEST ${partviewer_SRCS})
target_link_libraries(partviewer  ${KDE4_KPARTS_LIBS} Qt5::Test ${KCoreAddons_LIBRARIES} )

########### test prog for "open or save question" ###############

kde4_add_executable(openorsavequestion TEST openorsavequestion.cpp)
target_link_libraries(openorsavequestion  ${KDE4_KPARTS_LIBS} Qt5::Test ${KCoreAddons_LIBRARIES} )

########### next target ###############

set(spellcheckplugin_PART_SRCS plugin_spellcheck.cpp )

kde4_add_plugin(spellcheckplugin ${spellcheckplugin_PART_SRCS})

target_link_libraries(spellcheckplugin Qt5::Test ${KDE4_KPARTS_LIBS} )

install(TARGETS spellcheckplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############

set(notepadpart_PART_SRCS notepad.cpp)


kde4_add_plugin(notepadpart ${notepadpart_PART_SRCS})

target_link_libraries(notepadpart  ${KDE4_KPARTS_LIBS} Qt5::Test ${KCoreAddons_LIBRARIES} )

install(TARGETS notepadpart  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### unit tests ###############

MACRO(KPARTS_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} TESTNAME kparts-${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KPARTS_LIBS} Qt5::Test ${KCoreAddons_LIBRARIES} )
       ENDFOREACH(_testname)
ENDMACRO(KPARTS_UNIT_TESTS)
MACRO(KPARTS_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_executable(${_testname} TEST ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KPARTS_LIBS} Qt5::Test)
       ENDFOREACH(_testname)
ENDMACRO(KPARTS_EXECUTABLE_TESTS)

set(kparts_unittests
  parttest
  openorsavequestion_unittest
)
KPARTS_UNIT_TESTS(${kparts_unittests})

########### install files ###############

# Normally this would be needed. For the test programs we can avoid that, see addResourceDir.
#install( FILES notepadpart.rc  DESTINATION  ${DATA_INSTALL_DIR}/notepadpart )
#install( FILES partviewer_shell.rc  DESTINATION  ${DATA_INSTALL_DIR}/partviewer )
#install( FILES kpartstest_shell.rc  DESTINATION  ${DATA_INSTALL_DIR}/kpartstest )
#install( FILES kpartstest_part1.rc  DESTINATION  ${DATA_INSTALL_DIR}/kpartstestpart )

# To test plugins without installing them, try "ln -s kpartplugins <srcdir>"
#install( FILES plugin_foobar.rc plugin_spellcheck.rc  DESTINATION  ${DATA_INSTALL_DIR}/notepadpart/kpartplugins )
