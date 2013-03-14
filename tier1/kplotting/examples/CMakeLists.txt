remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src
                    ${CMAKE_CURRENT_BINARY_DIR}/../src)

set(kplottest_SRCS testplot_widget.cpp testplot_main.cpp)
add_executable(kplottest ${kplottest_SRCS})
target_link_libraries(kplottest KPlotting Qt5::Widgets)
