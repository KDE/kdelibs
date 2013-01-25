remove_definitions(-DQT_NO_CAST_FROM_ASCII)

find_package(Qt5Transitional REQUIRED MODULE COMPONENTS Widgets)

macro(KCOREADDONS_EXAMPLES)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp) # TODO NOGUI
        target_link_libraries(${_testname} ${QT_QTTEST_LIBRARY} ${QT_QTNETWORK_LIBRARY} KCoreAddons)
    endforeach()
endmacro()

########### kdirwatchtest_gui ###############

add_executable(kdirwatchtest_gui kdirwatchtest_gui.cpp)
target_link_libraries(kdirwatchtest_gui Qt5::Widgets KCoreAddons)
