set(PARTS_TEST_LIBRARY_DEPENDENCIES KParts Qt5::Test KCoreAddons KI18n XmlGui)

MACRO(KPARTS_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} TESTNAME kparts-${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${PARTS_TEST_LIBRARY_DEPENDENCIES})
       ENDFOREACH(_testname)
ENDMACRO(KPARTS_UNIT_TESTS)

KPARTS_UNIT_TESTS(
  parttest
  openorsavequestion_unittest
)
