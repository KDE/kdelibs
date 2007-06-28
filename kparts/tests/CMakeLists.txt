
include_directories( ${KDE4_KPARTS_INCLUDES} )

########### next target ###############

set(kpartstest_SRCS
example.cpp
parts.cpp
notepad.cpp
)

kde4_automoc(${kpartstest_SRCS})

kde4_add_test_executable(kpartstest ${kpartstest_SRCS})

target_link_libraries(kpartstest  ${KDE4_KPARTS_LIBS} )


########### next target ###############

set(normalktmtest_SRCS
normalktm.cpp
parts.cpp
notepad.cpp
)

kde4_automoc(${normalktmtest_SRCS})

kde4_add_test_executable(normalktmtest ${normalktmtest_SRCS})

target_link_libraries(normalktmtest  ${KDE4_KPARTS_LIBS} )


########### next target ###############

set(ghostviewtest_SRCS
ghostview.cpp
)

kde4_automoc(${ghostviewtest_SRCS})

kde4_add_test_executable(ghostviewtest ${ghostviewtest_SRCS})

target_link_libraries(ghostviewtest  ${KDE4_KPARTS_LIBS} )


########### next target ###############

set(spellcheckplugin_PART_SRCS plugin_spellcheck.cpp )

kde4_automoc(${spellcheckplugin_PART_SRCS})

kde4_add_plugin(spellcheckplugin WITH_PREFIX ${spellcheckplugin_PART_SRCS})

target_link_libraries(spellcheckplugin  ${KDE4_KPARTS_LIBS} )

install(TARGETS spellcheckplugin  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############

set(notepadpart_PART_SRCS notepadpart.cpp notepad.cpp)

kde4_automoc(${notepadpart_PART_SRCS})

kde4_add_plugin(notepadpart WITH_PREFIX ${notepadpart_PART_SRCS})

target_link_libraries(notepadpart  ${KDE4_KPARTS_LIBS} )
set_target_properties(notepadpart PROPERTIES COMPILE_FLAGS -DKDESRCDIR="\\"${CMAKE_CURRENT_SOURCE_DIR}\\"")

install(TARGETS notepadpart  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### unit tests ###############

MACRO(KPARTS_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_automoc(${_testname}.cpp)
               kde4_add_unit_test(${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KPARTS_LIBS} ${QT_QTTEST_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(KPARTS_UNIT_TESTS)
MACRO(KPARTS_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_automoc(${_testname}.cpp)
               kde4_add_test_executable(${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KPARTS_LIBS} ${QT_QTTEST_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(KPARTS_EXECUTABLE_TESTS)

set(kparts_unittests
  parttest
)
KPARTS_UNIT_TESTS(${kparts_unittests})

########### install files ###############

# Normally this would be needed. For the test programs we can avoid that, see addResourceDir.
#install( FILES plugin_foobar.rc plugin_spellcheck.rc  DESTINATION  ${DATA_INSTALL_DIR}/notepadpart/kpartplugins )
#install( FILES notepadpart.rc  DESTINATION  ${DATA_INSTALL_DIR}/notepadpart )
#install( FILES ghostviewtest_shell.rc  DESTINATION  ${DATA_INSTALL_DIR}/ghostviewtest )
#install( FILES kpartstest_shell.rc  DESTINATION  ${DATA_INSTALL_DIR}/kpartstest )
#install( FILES kpartstest_part1.rc  DESTINATION  ${DATA_INSTALL_DIR}/kpartstestpart )
