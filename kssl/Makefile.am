INCLUDES=$(all_includes) $(SSL_INCLUDES)

lib_LTLIBRARIES = libkssl.la
KSSLVERSION=	2.0.0
KSSLPATCH=	0

include_HEADERS = \
		kssl.h                   \
		ksslcertificatehome.h    \
		ksslsettings.h           \
		ksslcertificate.h        \
		ksslconnectioninfo.h     \
		ksslcertificatefactory.h \
		ksslpeerinfo.h

libkssl_la_SOURCES	= \
			kssl.cc                   \
			ksslcertificatehome.cc    \
			ksslsettings.cc           \
			ksslcertificate.cc        \
			ksslconnectioninfo.cc     \
			ksslcertificatefactory.cc \
			ksslpeerinfo.cc

libkssl_la_LDFLAGS = $(all_libraries) -version-info 2:0 -no-undefined
libkssl_la_LIBADD = $(LIBSSL) $(LIB_KDECORE) $(LIB_QT)

libkssl_la_METASOURCES = AUTO

