set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
########### next target ###############

# TODO automate and merge with test_filter?
set(test_suggest_SRCS test.cpp)


kde4_add_executable(test_suggest TEST ${test_suggest_SRCS})

target_link_libraries(test_suggest ${KDE4_KDECORE_LIBS} )


########### next target ###############

set(test_filter_SRCS test_filter.cpp )


kde4_add_unit_test(test_filter ${test_filter_SRCS})

target_link_libraries(test_filter ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

########### install files ###############


########### next target ###############

set(test_core_SRCS test_core.cpp )


kde4_add_unit_test(test_core ${test_core_SRCS})

target_link_libraries(test_core ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

########### install files ###############

