AM_CPPFLAGS = $(all_includes) $(SSL_INCLUDES)
kde_module_LTLIBRARIES = kcm_crypto.la

kcm_crypto_la_SOURCES = crypto.cpp certexport.cpp kdatetimedlg.cpp

kcm_crypto_la_LDFLAGS  = $(KDE_RPATH) $(all_libraries) $(SSL_LDFLAGS) -module -avoid-version -no-undefined
kcm_crypto_la_LIBADD = -lkdeui $(LIB_KIO) $(LIBSSL)

METASOURCES = AUTO

noinst_HEADERS = crypto.h certexport.h kdatetimedlg.h

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmcrypto.pot

xdg_apps_DATA = crypto.desktop
