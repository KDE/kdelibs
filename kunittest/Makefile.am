INCLUDES = $(all_includes)
METASOURCES = AUTO

lib_LTLIBRARIES = libkunittest.la
libkunittest_la_SOURCES = runner.cpp tester.cpp
libkunittest_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libkunittest_la_LIBADD = $(LIB_KDECORE)

libkunittestinclude_HEADERS = runner.h tester.h module.h
libkunittestincludedir = $(includedir)/kunittest

bin_PROGRAMS = kunittestmodrunner
kunittestmodrunner_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kunittestmodrunner_LDADD = libkunittest.la $(LIB_KDECORE)
kunittestmodrunner_SOURCES = modrunner.cpp

check_LTLIBRARIES = kunittest_samplemodule.la
kunittest_samplemodule_la_SOURCES = samplemodule.cpp
kunittest_samplemodule_la_LIBADD = libkunittest.la
kunittest_samplemodule_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) -rpath $(libdir)

# don't install the module, the KLibLoader will otherwise also pick up those
# also, it is not a good idea to pollute our poor users' hd with test-cases

TESTS = kunittestmodrunner
