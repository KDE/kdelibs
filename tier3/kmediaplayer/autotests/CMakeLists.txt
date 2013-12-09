find_package(Qt5Test ${QT_REQUIRED_VERSION} REQUIRED NO_MODULE)
include(ECMMarkAsTest)

macro(KMEDIAPLAYER_EXECUTABLE_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        target_link_libraries(${_testname} KF5::MediaPlayer)
        ecm_mark_as_test(${_testname})
    endforeach()
endmacro()

macro(KMEDIAPLAYER_UNIT_TESTS)
    foreach(_testname ${ARGN})
        kmediaplayer_executable_tests(${_testname})
        target_link_libraries(${_testname} Qt5::Test)
        add_test(kmediaplayer-${_testname} ${_testname})
        ecm_mark_as_test(${_testname})
    endforeach()
endmacro()

kmediaplayer_unit_tests(
   playertest
   viewtest
)

