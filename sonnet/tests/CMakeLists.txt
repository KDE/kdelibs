if(KDE4_BUILD_TESTS)

include_directories( ${CMAKE_SOURCE_DIR}/sonnet/ui )

# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

########### next target ###############

# TODO automate and merge with test_filter?
set(test_suggest_SRCS test.cpp)

kde4_automoc(${test_suggest_SRCS})

kde4_add_executable(test_suggest NOGUI RUN_UNINSTALLED ${test_suggest_SRCS})

target_link_libraries(test_suggest ${KDE4_KDECORE_LIBS} kspell2 )


########### next target ###############

set(test_filter_SRCS test_filter.cpp )

kde4_automoc(${test_filter_SRCS})

kde4_add_executable(test_filter NOGUI RUN_UNINSTALLED ${test_filter_SRCS})

target_link_libraries(test_filter ${KDE4_KDECORE_LIBS} kspell2  ${QT_QTTEST_LIBRARY})

add_test(kspell2_test_filter ${EXECUTABLE_OUTPUT_PATH}/test_filter)


########### next target ###############

set(backgroundtest_SRCS backgroundtest.cpp)

kde4_automoc(${backgroundtest_SRCS})

kde4_add_executable(backgroundtest RUN_UNINSTALLED ${backgroundtest_SRCS})

target_link_libraries(backgroundtest ${KDE4_KDECORE_LIBS} kspell2 )


########### next target ###############

set(test_dialog_SRCS test_dialog.cpp)

kde4_automoc(${test_dialog_SRCS})

kde4_add_executable(test_dialog RUN_UNINSTALLED ${test_dialog_SRCS})

target_link_libraries(test_dialog ${KDE4_KDECORE_LIBS} kspell2 )


########### next target ###############

set(test_highlighter_SRCS test_highlighter.cpp)

kde4_automoc(${test_highlighter_SRCS})

kde4_add_executable(test_highlighter RUN_UNINSTALLED ${test_highlighter_SRCS})

target_link_libraries(test_highlighter ${KDE4_KDECORE_LIBS} kspell2 )


########### next target ###############

set(test_configdialog_SRCS test_configdialog.cpp)

kde4_automoc(${test_configdialog_SRCS})

kde4_add_executable(test_configdialog RUN_UNINSTALLED ${test_configdialog_SRCS})

target_link_libraries(test_configdialog ${KDE4_KDECORE_LIBS} kspell2 )

endif(KDE4_BUILD_TESTS)

########### install files ###############





