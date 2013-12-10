set(kidletime_example_SRCS main.cpp KIdleTest.cpp)
add_executable(kidletime_example ${kidletime_example_SRCS})
target_link_libraries(kidletime_example KF5::IdleTime Qt5::Widgets)
