set(PARTS_TEST_LIBRARY_DEPENDENCIES KParts Qt5::Test KCoreAddons KI18n XmlGui)

MACRO(KPARTS_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
            add_executable(${_testname} ${_testname}.cpp)
            add_test(kparts-${_testname} ${_testname})
            ecm_mark_as_test(${_testname})
            target_link_libraries(${_testname} ${PARTS_TEST_LIBRARY_DEPENDENCIES})
       ENDFOREACH(_testname)
ENDMACRO(KPARTS_UNIT_TESTS)

KPARTS_UNIT_TESTS(
  parttest
  openorsavequestion_unittest
)
