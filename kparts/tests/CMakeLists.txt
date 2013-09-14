set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

########### next target ###############

set(PARTS_TEST_LIBRARY_DEPENDENCIES ${KDE4_KPARTS_LIBS} Qt5::Test ${KCoreAddons_LIBRARIES} KI18n XmlGui)

add_executable(kpartstest testmainwindow.cpp parts.cpp notepad.cpp)
ecm_mark_as_test(kpartstest)

target_link_libraries(kpartstest ${PARTS_TEST_LIBRARY_DEPENDENCIES})


########### next target ###############

set(normalktmtest_SRCS
normalktm.cpp
parts.cpp
notepad.cpp
)


add_executable(normalktmtest ${normalktmtest_SRCS})
ecm_mark_as_test(normalktmtest)
target_link_libraries(normalktmtest ${PARTS_TEST_LIBRARY_DEPENDENCIES})


########### next target ###############

add_executable(partviewer partviewer.cpp)
ecm_mark_as_test(partviewer)
target_link_libraries(partviewer ${PARTS_TEST_LIBRARY_DEPENDENCIES})

########### test prog for "open or save question" ###############

add_executable(openorsavequestion openorsavequestion.cpp)
ecm_mark_as_test(openorsavequestion)
target_link_libraries(openorsavequestion ${PARTS_TEST_LIBRARY_DEPENDENCIES})

########### next target ###############

set(spellcheckplugin_PART_SRCS plugin_spellcheck.cpp )

kde4_add_plugin(spellcheckplugin ${spellcheckplugin_PART_SRCS})

target_link_libraries(spellcheckplugin ${PARTS_TEST_LIBRARY_DEPENDENCIES} )

install(TARGETS spellcheckplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############

set(notepadpart_PART_SRCS notepad.cpp)


kde4_add_plugin(notepadpart ${notepadpart_PART_SRCS})

target_link_libraries(notepadpart ${PARTS_TEST_LIBRARY_DEPENDENCIES})

install(TARGETS notepadpart  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### unit tests ###############

MACRO(KPARTS_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} TESTNAME kparts-${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${PARTS_TEST_LIBRARY_DEPENDENCIES})
       ENDFOREACH(_testname)
ENDMACRO(KPARTS_UNIT_TESTS)
MACRO(KPARTS_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               add_executable(${_testname}  ${_testname}.cpp)
               ecm_mark_as_test(${_testname})
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
