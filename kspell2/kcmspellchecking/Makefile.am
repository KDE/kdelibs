kde_module_LTLIBRARIES = kcm_spellchecking.la

kcm_spellchecking_la_SOURCES = spellchecking.cpp

kcm_spellchecking_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kcm_spellchecking_la_LIBADD = $(LIB_KDEUI) $(LIB_KDECORE) ../libkspell2base.la

AM_CPPFLAGS= $(all_includes)

METASOURCES = spellchecking.moc


xdg_apps_DATA = spellchecking.desktop

