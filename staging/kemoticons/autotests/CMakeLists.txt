include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/../src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core
    ${CMAKE_SOURCE_DIR}/kdecore/util
    ${CMAKE_SOURCE_DIR}/kdeui/widgets
    ${KDE4_KDEUI_INCLUDES}
    ${CMAKE_BINARY_DIR}/staging/kwidgets/src
    ${CMAKE_SOURCE_DIR}/staging/kwidgets/src/utils
)

macro(kemoticons_unit_tests)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp)
      add_test(kemoticons-${_testname} ${_testname})
      target_link_libraries(${_testname} ${KDE4_KIO_LIBS} ${QT_QTTEST_LIBRARY} kcoreaddons kemoticons)
      ecm_mark_as_test(${_testname})
   endforeach()
endmacro()

kemoticons_unit_tests(kemoticontest)

