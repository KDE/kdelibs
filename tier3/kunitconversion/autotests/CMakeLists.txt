include(ECMMarkAsTest)

find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../src")
include_directories("${CMAKE_CURRENT_BINARY_DIR}/../src")

macro(KUNITCONVERSION_UNIT_TESTS)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp)
        add_test(kunitconversion-${_testname} ${_testname})
        ecm_mark_as_test(${_testname})
        target_link_libraries(${_testname} KF5::UnitConversion Qt5::Test)
    endforeach(_testname)
endmacro(KUNITCONVERSION_UNIT_TESTS)

KUNITCONVERSION_UNIT_TESTS(
    valuetest
    categorytest
    convertertest
)
