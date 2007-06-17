########### next target ###############

# TODO automate and merge with test_filter?
set(test_suggest_SRCS test.cpp)

kde4_automoc(${test_suggest_SRCS})

kde4_add_test_executable(test_suggest ${test_suggest_SRCS})

target_link_libraries(test_suggest ${KDE4_KDECORE_LIBS} )


########### next target ###############

set(test_filter_SRCS test_filter.cpp )

kde4_automoc(${test_filter_SRCS})

kde4_add_unit_test(test_filter ${test_filter_SRCS})

target_link_libraries(test_filter ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY})

########### install files ###############





