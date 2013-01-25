
remove_definitions(-DQT_NO_KEYWORDS)

set(kidletime_example_SRCS main.cpp KIdleTest.cpp)
add_executable(kidletime_example ${kidletime_example_SRCS})
target_link_libraries(kidletime_example KIdleTime Qt5::Widgets ${QT_QTDBUS_LIBRARY})
