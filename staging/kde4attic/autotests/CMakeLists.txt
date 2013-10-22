
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
  kcalendartest
  kdatetimetest
  klocaletimeformattest
  ktimezonestest
  ktimezonewidget_unittest
  ktcpsockettest
)

target_include_directories(ktimezonestest PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../src/date")

########### klocaletest ###############

# compile into the test since it's not exported
set(klocaletest_SRCS klocaletest.cpp ../src/date/kdayperiod.cpp)
add_executable(klocaletest ${klocaletest_SRCS})
add_test(kde4attic-klocaletest klocaletest)
ecm_mark_as_test(klocaletest)
target_link_libraries(klocaletest Qt5::Test KF5::KI18n KDE4Attic)

########### kdatetimeformattertest ###############

# compile KDateTimeFormatter and KDayPeriod into the test since it's not exported
set(kdatetimeformattertest_SRCS kdatetimeformattertest.cpp ../src/date/kdatetimeformatter.cpp ../src/date/kdayperiod.cpp)
add_executable(kdatetimeformattertest ${kdatetimeformattertest_SRCS})
add_test(kde4attic-kdatetimeformattertest kdatetimeformattertest)
ecm_mark_as_test(kdatetimeformattertest)
target_link_libraries(kdatetimeformattertest Qt5::Test KF5::KI18n KDE4Attic)
