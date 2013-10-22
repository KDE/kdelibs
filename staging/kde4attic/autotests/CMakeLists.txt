
find_package(Qt5 5.2.0 REQUIRED NO_MODULE COMPONENTS Test)

include(ECMMarkAsTest)

macro(KDE4ATTIC_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        add_test(kde4attic-${_testname} ${_testname})
        ecm_mark_as_test(${_testname})
        target_link_libraries(${_testname} Qt5::Test KDE4Attic KF5::KI18n)
    endforeach(_testname)
endmacro()

kde4attic_tests(
  ktcpsockettest
)

