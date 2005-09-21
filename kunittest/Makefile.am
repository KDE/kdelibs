INCLUDES = $(all_includes)
METASOURCES = AUTO

lib_LTLIBRARIES = libkunittest.la
libkunittest_la_SOURCES = runner.cpp tester.cpp
libkunittest_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -version-info 1:0:0
libkunittest_la_LIBADD = $(LIB_KDECORE)

libkunittestinclude_HEADERS = runner.h tester.h module.h
libkunittestincludedir = $(includedir)/kunittest

bin_PROGRAMS = kunittestmodrunner
kunittestmodrunner_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kunittestmodrunner_LDADD = libkunittest.la $(LIB_KDECORE)
kunittestmodrunner_SOURCES = modrunner.cpp

# The check_ target makes sure we don't install the modules,
# $(KDE_CHECK_PLUGIN) assures a shared library is created.
check_LTLIBRARIES = kunittest_samplemodule.la
kunittest_samplemodule_la_SOURCES = samplemodule.cpp
kunittest_samplemodule_la_LIBADD = libkunittest.la
kunittest_samplemodule_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)

check-local:
	./kunittestmodrunner

include ../admin/Doxyfile.am
