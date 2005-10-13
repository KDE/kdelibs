kde_module_LTLIBRARIES = kcm_spellchecking.la

kcm_spellchecking_la_SOURCES = spellchecking.cpp

kcm_spellchecking_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kcm_spellchecking_la_LIBADD = -lkdecore -lkdeui ../libkspell2base.la -L../

AM_CPPFLAGS= $(all_includes)

METASOURCES = spellchecking.moc

messages:
	$(XGETTEXT) $(kcm_spellchecking_la_SOURCES) -o $(podir)/kcmspellchecking.pot

xdg_apps_DATA = spellchecking.desktop

