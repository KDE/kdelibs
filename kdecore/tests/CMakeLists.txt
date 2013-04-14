set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMMarkAsTest)

MACRO(KDECORE_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} TESTNAME "kdecore-${_testname}" NOGUI ${_testname}.cpp)
               target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} Qt5::Test Qt5::Network ${KCoreAddons_LIBRARIES}  kdeqt5staging)
               if(WINCE)
                target_link_libraries(${_testname} ${WCECOMPAT_LIBRARIES})
               endif(WINCE)
       ENDFOREACH(_testname)
ENDMACRO(KDECORE_UNIT_TESTS)
MACRO(KDECORE_EXECUTABLE_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_executable(${_testname} NOGUI ${_testname}.cpp)
               ecm_mark_as_test(${_testname})
               target_link_libraries(${_testname} ${KDE4_KDECORE_LIBS} Qt5::Test ${KCoreAddons_LIBRARIES} )
               if(WINCE)
                target_link_libraries(${_testname} ${WCECOMPAT_LIBRARIES})
               endif(WINCE)
       ENDFOREACH(_testname)
ENDMACRO(KDECORE_EXECUTABLE_TESTS)


########### next target ###############

KDECORE_UNIT_TESTS(
 klocaletimeformattest
 kmountpointtest
 kurltest
 cplusplustest
 ksortablelisttest
 kshelltest
 ktimezonestest
 ksharedptrtest
 kdatetimetest
 kautostarttest
 kprocesstest
 ktcpsockettest
 qcoreapptest
 kcalendartest
 kconfignokdehometest
)

target_include_directories(ktimezonestest PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../date")

KDECORE_EXECUTABLE_TESTS(
 kmemtest
 dbuscalltest
 startserviceby
)

########### klocaletest ###############

# compile into the test since it's not exported
set(klocaletest_SRCS klocaletest.cpp ../date/kdayperiod.cpp)
kde4_add_unit_test(klocaletest ${klocaletest_SRCS})
target_link_libraries(klocaletest ${KDE4_KDECORE_LIBS} Qt5::Test ${KCoreAddons_LIBRARIES})

########### kdatetimeformattertest ###############

# compile KDateTimeFormatter and KDayPeriod into the test since it's not exported
set(kdatetimeformattertest_SRCS kdatetimeformattertest.cpp ../date/kdatetimeformatter.cpp ../date/kdayperiod.cpp)
kde4_add_unit_test(kdatetimeformattertest ${kdatetimeformattertest_SRCS})
target_link_libraries(kdatetimeformattertest ${KDE4_KDECORE_LIBS} Qt5::Test ${KCoreAddons_LIBRARIES})

########### kcompositejobtest ###############

set(kcompositejobtest_SRCS kcompositejobtest.cpp)
kde4_add_unit_test(kcompositejobtest TESTNAME kdecore-kcompositejobtest ${kcompositejobtest_SRCS})
target_link_libraries(kcompositejobtest ${KDE4_KDECORE_LIBS} Qt5::Test )
