METASOURCES = AUTO

AM_CPPFLAGS = -I$(top_srcdir)/kspell2 -I$(top_srcdir) $(all_includes)

# For the future: examine if condensing the tons of *_LDFLAGS variables
# into $(all_libraries) isn't better
AM_LDFLAGS = $(LDFLAGS_AS_NEEDED) $(LDFLAGS_NEW_DTAGS)

kde_module_LTLIBRARIES = kspell_aspell.la

kspell_aspell_la_SOURCES = kspell_aspellclient.cpp kspell_aspelldict.cpp

kspell_aspell_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN)
kspell_aspell_la_LIBADD = ../../ui/libkspell2.la -laspell

service_DATA = kspell_aspell.desktop
servicedir = $(kde_servicesdir)
