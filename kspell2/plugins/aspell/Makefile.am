METASOURCES = AUTO

AM_CPPFLAGS = -I$(top_srcdir)/kspell2 -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = kspell_aspell.la

kspell_aspell_la_SOURCES = kspell_aspellclient.cpp kspell_aspelldict.cpp

kspell_aspell_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN)
kspell_aspell_la_LIBADD = ../../libkspell2.la -laspell

service_DATA = kspell_aspell.desktop
servicedir = $(kde_servicesdir)
