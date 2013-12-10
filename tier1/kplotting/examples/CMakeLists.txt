remove_definitions(-DQT_NO_CAST_FROM_ASCII)

set(kplottest_SRCS testplot_widget.cpp testplot_main.cpp)
add_executable(kplottest ${kplottest_SRCS})

target_link_libraries(kplottest KF5::Plotting Qt5::Widgets)
