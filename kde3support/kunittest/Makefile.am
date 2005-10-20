INCLUDES = -I$(top_srcdir)/kde3support $(all_includes)
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

# The check_ target makes sure we don't install the modules,
# -rpath $(libdir) assures a shared library is created (used to be done by $(KDE_CHECK_PLUGIN)).
check_LTLIBRARIES = kunittest_samplemodule.la
kunittest_samplemodule_la_SOURCES = samplemodule.cpp
kunittest_samplemodule_la_LIBADD = libkunittest.la
kunittest_samplemodule_la_LDFLAGS = -module $(KDE_PLUGIN) -rpath $(libdir) $(all_libraries)

check-local:
	./kunittestmodrunner

include $(top_srcdir)/admin/Doxyfile.am
