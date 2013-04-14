remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMMarkAsTest)

########### test_suggest ###############
# TODO automate and merge with test_filter?
set(test_suggest_SRCS test.cpp)
add_executable(test_suggest ${test_suggest_SRCS})
target_link_libraries(test_suggest sonnetcore)
ecm_mark_as_test(test_suggest)

########### test_dialog ###############

set(test_dialog_SRCS test_dialog.cpp)
add_executable(test_dialog ${test_dialog_SRCS})
target_link_libraries(test_dialog Qt5::Widgets sonnetcore sonnetui)
ecm_mark_as_test(test_dialog)
