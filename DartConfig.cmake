# support for dart:  http://public.kitware.com/KDE/Testing 
# submitted by Bill Hoffman
SET (NIGHTLY_START_TIME "21:00:00 EDT")
SET(DROP_METHOD http)
SET (DROP_SITE "public.kitware.com")
SET (DROP_LOCATION "/cgi-bin/HTTPUploadDartFile.cgi")
SET (TRIGGER_SITE "http://${DROP_SITE}/cgi-bin/SubmitKDE.cgi")
