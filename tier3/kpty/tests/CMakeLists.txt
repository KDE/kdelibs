include(ECMMarkAsTest)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

add_executable(kptyprocesstest kptyprocesstest.cpp)
target_link_libraries(kptyprocesstest KPty Qt5::Test KF5::KCoreAddons )
ecm_mark_as_test(kptyprocesstest)