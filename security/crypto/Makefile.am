INCLUDES = $(SSL_INCLUDES) $(all_includes)
kde_module_LTLIBRARIES = kcm_crypto.la

kcm_crypto_la_SOURCES = crypto.cpp certexport.cpp kdatetimedlg.cpp

kcm_crypto_la_LDFLAGS  = $(KDE_RPATH) $(SSL_LDFLAGS) $(all_libraries) -module -avoid-version -no-undefined
kcm_crypto_la_LIBADD = -lkdeui $(LIB_KIO) $(LIBSSL)

kcm_crypto_la_METASOURCES = AUTO

noinst_HEADERS = crypto.h certexport.h kdatetimedlg.h

KDE_ICON = crypto

messages: rc.cpp
	: > rc.cpp
	$(XGETTEXT) $(kcm_crypto_la_SOURCES) -o $(podir)/kcmcrypto.pot

data_DATA = crypto.desktop
datadir = $(kde_appsdir)/Settings/Security
