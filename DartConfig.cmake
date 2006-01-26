# support for dart:  http://public.kitware.com/KDE/Testing 
# submitted by Bill Hoffman
include(${CMAKE_CURRENT_SOURCE_DIR}/CTestConfig.cmake)
set(NIGHTLY_START_TIME "${CTEST_NIGHTLY_START_TIME}")
set(DROP_METHOD "${CTEST_DROP_METHOD}")
set(DROP_SITE "${CTEST_DROP_SITE}")
set(DROP_LOCATION "${CTEST_DROP_LOCATION}")
set(TRIGGER_SITE "${CTEST_TRIGGER_SITE}")
