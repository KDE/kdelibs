if(KDE4_BUILD_TESTS)

# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

########### next target ###############

# TODO automate and merge with test_filter?
set(test_suggest_SRCS test.cpp)

kde4_automoc(${test_suggest_SRCS})

kde4_add_executable(test_suggest NOGUI RUN_UNINSTALLED ${test_suggest_SRCS})

target_link_libraries(test_suggest ${KDE4_KDECORE_LIBS} )


########### next target ###############

set(test_filter_SRCS test_filter.cpp )

kde4_automoc(${test_filter_SRCS})

kde4_add_executable(test_filter NOGUI RUN_UNINSTALLED ${test_filter_SRCS})

target_link_libraries(test_filter ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

add_test(kspell2_test_filter ${EXECUTABLE_OUTPUT_PATH}/test_filter)


endif(KDE4_BUILD_TESTS)

########### install files ###############





