INCLUDES = $(all_includes) $(SSL_INCLUDES)
kde_module_LTLIBRARIES = libkcm_crypto.la

libkcm_crypto_la_SOURCES = crypto.cpp certexport.cpp kdatetimedlg.cpp

libkcm_crypto_la_LDFLAGS  = $(all_libraries) $(SSL_LDFLAGS) -module -avoid-version -no-undefined
libkcm_crypto_la_LIBADD = -lkdeui -lkfile $(LIBSSL)

libkcm_crypto_la_METASOURCES = AUTO

noinst_HEADERS = crypto.h certexport.h kdatetimedlg.h

KDE_ICON = crypto

messages: rc.cpp
	: > rc.cpp
	$(XGETTEXT) $(libkcm_crypto_la_SOURCES) -o $(podir)/kcmcrypto.pot

data_DATA = crypto.desktop
datadir = $(kde_appsdir)/Settings/Personalization
