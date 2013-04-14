remove_definitions(-DQT_NO_CAST_FROM_ASCII)

find_package(Qt5Transitional REQUIRED MODULE COMPONENTS Widgets)

macro(KCOREADDONS_EXAMPLES)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp) # TODO NOGUI
        target_link_libraries(${_testname} Qt5::Test Qt5::Network KCoreAddons)
    endforeach()
endmacro()

########### kdirwatchtest_gui ###############

add_executable(kdirwatchtest_gui kdirwatchtest_gui.cpp)
target_link_libraries(kdirwatchtest_gui Qt5::Widgets KCoreAddons)
