## This file should be placed in the root directory of your project.
## Then modify the CMakeLists.txt file in the root directory of your
## project to incorporate the testing dashboard.
## # The following are required to uses Dart and the Cdash dashboard
##   ENABLE_TESTING()
##   INCLUDE(Dart)
set(CTEST_PROJECT_NAME "kdelibs")
set(CTEST_NIGHTLY_START_TIME "20:00:00 CET")

set(CTEST_DROP_METHOD "http")
set(CTEST_DROP_SITE "my.cdash.org")
set(CTEST_DROP_LOCATION "/submit.php?project=kdelibs")
set(CTEST_DROP_SITE_CDASH TRUE)

set(CTEST_CUSTOM_MAXIMUM_NUMBER_OF_WARNINGS 10000)
set(CTEST_CUSTOM_WARNING_EXCEPTION "kdecore/network/k3socket[a-z]+\\.h"
                                   "kdecore/network/k3clientsocketbase\\.h"
                                   "Warning: No relevant classes found. No output generated."  # moc didn't find anything to do
                                   "yy" "YY"                                                   # ignore the warnings in generated code
   )
set(CTEST_CUSTOM_ERROR_EXCEPTION "ICECC")
set(CTEST_CUSTOM_COVERAGE_EXCLUDE ".moc$" "moc_" "ui_")
