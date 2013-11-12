include(ECMMarkAsTest)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

add_executable(kptyprocesstest kptyprocesstest.cpp)
target_link_libraries(kptyprocesstest KPty Qt5::Test)
ecm_mark_as_test(kptyprocesstest)
