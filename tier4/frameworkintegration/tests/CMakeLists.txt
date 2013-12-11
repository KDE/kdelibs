add_executable(kstyletest kstyletest.cpp)
target_link_libraries(kstyletest Qt5::Widgets KF5::Style)

add_executable(qfiledialogtest qfiledialogtest.cpp)
target_link_libraries(qfiledialogtest Qt5::Core Qt5::Widgets)
