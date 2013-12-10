remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)

find_package(Qt5 5.2.0 CONFIG REQUIRED Test)

include(ECMMarkAsTest)

macro(KCOMPLETION_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        add_test(kcompletion-${_testname} ${_testname})
        ecm_mark_as_test(${_testname})
        target_link_libraries(${_testname} Qt5::Test KF5::Completion)
    endforeach(_testname)
endmacro()

kcompletion_tests(
   kcompletioncoretest
   klineedit_unittest
   kcombobox_unittest
   ksortablelisttest
)
