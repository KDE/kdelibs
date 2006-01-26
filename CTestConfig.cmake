# support for dart: http://public.kitware.com/KDE/Testing 
# submitted by Bill Hoffman
set (CTEST_PROJECT_NAME "KDE")
set (CTEST_NIGHTLY_START_TIME "21:00:00 EDT")
set (CTEST_DROP_METHOD "http")
set (CTEST_DROP_SITE "public.kitware.com")
set (CTEST_DROP_LOCATION "/cgi-bin/HTTPUploadDartFile.cgi")
set (CTEST_TRIGGER_SITE "http://${CTEST_DROP_SITE}/cgi-bin/SubmitKDE.cgi")
