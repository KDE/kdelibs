METASOURCES = AUTO

AM_CPPFLAGS = -I$(top_srcdir)/kspell2 -I$(top_srcdir) $(all_includes)

# For the future: examine if condensing the tons of *_LDFLAGS variables
# into $(all_libraries) isn't better
AM_LDFLAGS = $(LDFLAGS_AS_NEEDED) $(LDFLAGS_NEW_DTAGS)

kde_module_LTLIBRARIES = kspell_hspell.la

kspell_hspell_la_SOURCES = kspell_hspellclient.cpp kspell_hspelldict.cpp

kspell_hspell_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN)
kspell_hspell_la_LIBADD = ../../ui/libkspell2.la -lhspell -lz

service_DATA = kspell_hspell.desktop
servicedir = $(kde_servicesdir)
