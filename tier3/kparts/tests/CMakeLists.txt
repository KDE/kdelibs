set(PARTS_TEST_LIBRARY_DEPENDENCIES Qt5::Test KParts KCoreAddons KI18n XmlGui KIOCore)

set(partstest_SRCS
    parts.cpp
    notepad.cpp
)


add_executable(kpartstest testmainwindow.cpp ${partstest_SRCS})
ecm_mark_as_test(kpartstest)
target_link_libraries(kpartstest ${PARTS_TEST_LIBRARY_DEPENDENCIES})

add_executable(normalktmtest normalktm.cpp ${partstest_SRCS})
ecm_mark_as_test(normalktmtest)
target_link_libraries(normalktmtest ${PARTS_TEST_LIBRARY_DEPENDENCIES})


########### next target ###############

add_library(spellcheckplugin MODULE plugin_spellcheck.cpp)
target_link_libraries(spellcheckplugin ${PARTS_TEST_LIBRARY_DEPENDENCIES} )
install(TARGETS spellcheckplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### next target ###############

add_library(notepadpart MODULE notepad.cpp)
target_link_libraries(notepadpart ${PARTS_TEST_LIBRARY_DEPENDENCIES})
install(TARGETS notepadpart  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### unit tests ###############

MACRO(KPARTS_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               add_executable(${_testname}  ${_testname}.cpp)
               ecm_mark_as_test(${_testname})
               target_link_libraries(${_testname} ${PARTS_TEST_LIBRARY_DEPENDENCIES})
       ENDFOREACH(_testname)
ENDMACRO(KPARTS_EXECUTABLE_TESTS)

KPARTS_EXECUTABLE_TESTS(
    partviewer
    openorsavequestion
)

########### install files ###############

# Normally this would be needed. For the test programs we can avoid that, see addResourceDir.
#install( FILES notepadpart.rc  DESTINATION  ${DATA_INSTALL_DIR}/notepadpart )
#install( FILES partviewer_shell.rc  DESTINATION  ${DATA_INSTALL_DIR}/partviewer )
#install( FILES kpartstest_shell.rc  DESTINATION  ${DATA_INSTALL_DIR}/kpartstest )
#install( FILES kpartstest_part1.rc  DESTINATION  ${DATA_INSTALL_DIR}/kpartstestpart )

# To test plugins without installing them, try "ln -s kpartplugins <srcdir>"
#install( FILES plugin_foobar.rc plugin_spellcheck.rc  DESTINATION  ${DATA_INSTALL_DIR}/notepadpart/kpartplugins )
