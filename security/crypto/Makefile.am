INCLUDES = $(all_includes) $(SSL_INCLUDES)
lib_LTLIBRARIES = libkcm_crypto.la

libkcm_crypto_la_SOURCES = crypto.cpp 

libkcm_crypto_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
libkcm_crypto_la_LIBADD = -lkdeui -lkfile $(LIBSSL)
INCLUDES= $(all_includes)

libkcm_crypto_la_METASOURCES = AUTO

noinst_HEADERS = crypto.h

KDE_ICON = crypto

messages:
	$(XGETTEXT) $(libkcm_crypto_la_SOURCES) -o $(podir)/kcmcrypto.pot

data_DATA = crypto.desktop
datadir = $(kde_appsdir)/Settings/Personalization
