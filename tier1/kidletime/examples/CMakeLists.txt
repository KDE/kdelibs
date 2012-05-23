include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src
                    ${CMAKE_CURRENT_BINARY_DIR}/../src)

remove_definitions(-DQT_NO_KEYWORDS)

set(kidletime_example_SRCS main.cpp KIdleTest.cpp)
add_executable(kidletime_example ${kidletime_example_SRCS})
target_link_libraries(kidletime_example kidletime ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY})
