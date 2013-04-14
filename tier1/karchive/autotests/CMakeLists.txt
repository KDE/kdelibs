remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMMarkAsTest)

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Test Network)

macro(KARCHIVE_UNIT_TESTS)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp) # TODO NOGUI
      add_test(karchive-${_testname} ${_testname})
      target_link_libraries(${_testname} KArchive ${QT_QTTEST_LIBRARY} ${QT_QTNETWORK_LIBRARY})
      ecm_mark_as_test(${_testname})
   endforeach(_testname)
endmacro(KARCHIVE_UNIT_TESTS)

karchive_unit_tests(
  karchivetest
)

########### klimitediodevicetest ###############

add_executable(klimitediodevicetest klimitediodevicetest.cpp ../src/klimitediodevice.cpp)
add_test(karchive-klimitediodevicetest klimitediodevicetest)
target_link_libraries(klimitediodevicetest ${QT_QTTEST_LIBRARY} )
target_include_directories(klimitediodevicetest PRIVATE $<TARGET_PROPERTY:KArchive,INTERFACE_INCLUDE_DIRECTORIES>)

########### kfiltertest ###############

add_executable(kfiltertest kfiltertest.cpp)
add_test(karchive-kfiltertest kfiltertest)
target_link_libraries(kfiltertest ${QT_QTTEST_LIBRARY} KArchive ${ZLIB_LIBRARIES})
