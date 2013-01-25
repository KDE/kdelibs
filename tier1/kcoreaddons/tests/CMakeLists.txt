remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../src/randomness
   ${CMAKE_CURRENT_SOURCE_DIR}/../src/io
   ${CMAKE_CURRENT_BINARY_DIR}/../src
)

find_package(Qt5Transitional REQUIRED MODULE COMPONENTS Gui)

macro(KCOREADDONS_EXAMPLES)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp) # TODO NOGUI
        target_link_libraries(${_testname} ${QT_QTTEST_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTMAIN_LIBRARY} kcoreaddons)
        if(WINCE)
            target_link_libraries(${_testname} ${WCECOMPAT_LIBRARIES})
        endif()
    endforeach()
endmacro()

kcoreaddons_examples(
  kdirwatchtest
  krandomsequencetest
)

########### kdirwatchtest_gui ###############

add_executable(kdirwatchtest_gui kdirwatchtest_gui.cpp)
target_link_libraries(kdirwatchtest_gui ${QT_QTGUI_LIBRARY} ${QT_QTMAIN_LIBRARY} kcoreaddons)

