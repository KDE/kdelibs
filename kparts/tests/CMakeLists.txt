
if(KDE4_BUILD_TESTS)

# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${KDE4_KPARTS_INCLUDES} )

add_definitions( -DKDESRCDIR=\\"${CMAKE_CURRENT_SOURCE_DIR}\\" )

########### next target ###############

set(kpartstest_SRCS
example.cpp
parts.cpp
notepad.cpp
)

kde4_automoc(${kpartstest_SRCS})

kde4_add_executable(kpartstest NOGUI ${kpartstest_SRCS})

target_link_libraries(kpartstest  ${KDE4_KPARTS_LIBS} )


########### next target ###############

set(normalktmtest_SRCS
normalktm.cpp
parts.cpp
notepad.cpp
)

kde4_automoc(${normalktmtest_SRCS})

kde4_add_executable(normalktmtest NOGUI ${normalktmtest_SRCS})

target_link_libraries(normalktmtest  ${KDE4_KPARTS_LIBS} )


########### next target ###############

set(ghostviewtest_SRCS
ghostview.cpp
)

kde4_automoc(${ghostviewtest_SRCS})

kde4_add_executable(ghostviewtest NOGUI ${ghostviewtest_SRCS})

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

install(TARGETS notepadpart  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### unit tests ###############

MACRO(KPARTS_BUILD_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_automoc(${_testname}.cpp)
               kde4_add_test(${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KPARTS_LIBS} ${QT_QTTEST_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(KPARTS_BUILD_TESTS)
MACRO(KPARTS_EXECUTE_TESTS)
       FOREACH(_testname ${ARGN})
               add_test(kparts-${_testname} ${EXECUTABLE_OUTPUT_PATH}/${_testname})
       ENDFOREACH(_testname)
ENDMACRO(KPARTS_EXECUTE_TESTS)

set(kparts_unittests
  parttest
)
KPARTS_BUILD_TESTS(${kparts_unittests})
KPARTS_EXECUTE_TESTS(${kparts_unittests})

########### install files ###############

# Normally this would be needed. For the test programs we can avoid that, see addResourceDir.
#install( FILES plugin_foobar.rc plugin_spellcheck.rc  DESTINATION  ${DATA_INSTALL_DIR}/notepadpart/kpartplugins )
#install( FILES notepadpart.rc  DESTINATION  ${DATA_INSTALL_DIR}/notepadpart )
#install( FILES ghostviewtest_shell.rc  DESTINATION  ${DATA_INSTALL_DIR}/ghostviewtest )
#install( FILES kpartstest_shell.rc  DESTINATION  ${DATA_INSTALL_DIR}/kpartstest )
#install( FILES kpartstest_part1.rc  DESTINATION  ${DATA_INSTALL_DIR}/kpartstestpart )

endif(KDE4_BUILD_TESTS)
