INCLUDES = $(all_includes)

noinst_HEADERS = highcontrastconfig.h
kde_module_LTLIBRARIES = kstyle_highcontrast_config.la
kstyle_highcontrast_config_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
kstyle_highcontrast_config_la_LIBADD  = $(LIB_KDEUI)
kstyle_highcontrast_config_la_SOURCES = highcontrastconfig.cpp
kstyle_highcontrast_config_la_METASOURCES = AUTO


messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/kstyle_highcontrast_config.pot
