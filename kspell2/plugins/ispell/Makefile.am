METASOURCES = AUTO

AM_CPPFLAGS = -I$(top_srcdir)/kspell2 -I$(top_srcdir) $(all_includes)

# For the future: examine if condensing the tons of *_LDFLAGS variables
# into $(all_libraries) isn't better
AM_LDFLAGS = $(LDFLAGS_AS_NEEDED) $(LDFLAGS_NEW_DTAGS)

kde_module_LTLIBRARIES = kspell_ispell.la

kspell_ispell_la_SOURCES = kspell_ispellclient.cpp kspell_ispelldict.cpp \
	correct.cpp		\
	good.cpp		\
	hash.cpp		\
	lookup.cpp		\
	makedent.cpp		\
	tgood.cpp		\
	ispell_checker.cpp

kspell_ispell_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN)
kspell_ispell_la_LIBADD = ../../ui/libkspell2.la 

service_DATA = kspell_ispell.desktop
servicedir = $(kde_servicesdir)
