INCLUDES=$(all_includes) $(SSL_INCLUDES)

lib_LTLIBRARIES = libkssl.la
KSSLVERSION=	2.9.8
KSSLPATCH=	0
METASOURCES= AUTO

include_HEADERS = \
		kssl.h                   \
		ksslcertificatehome.h    \
		ksslsettings.h           \
		ksslcertificate.h        \
		ksslconnectioninfo.h     \
		ksslcertificatefactory.h \
		ksslcertificatecache.h   \
		ksslpeerinfo.h           \
		ksslinfodlg.h            \
		ksslcertdlg.h            \
		ksslutils.h              \
		kopenssl.h               \
		ksslall.h	 	 \
		ksslpkcs12.h		 \
		ksslcertchain.h		 \
		ksslkeygen.h		 \
		ksslx509v3.h		 \
		ksslsigners.h		 \
		ksslx509map.h
 

libkssl_la_SOURCES	= \
			kssl.cc                   \
			ksslcertificatehome.cc    \
			ksslsettings.cc           \
			ksslcertificate.cc        \
			ksslconnectioninfo.cc     \
			ksslcertificatefactory.cc \
			ksslcertificatecache.cc   \
			ksslpeerinfo.cc           \
			ksslinfodlg.cc            \
			ksslcertdlg.cc            \
			ksslutils.cc              \
			kopenssl.cc		  \
			ksslpkcs12.cc		  \
			ksslcertchain.cc	  \
			ksslkeygen.cc		  \
			ksslx509v3.cc		  \
			ksslsigners.cc		  \
			ksslx509map.cc

libkssl_la_LDFLAGS = $(all_libraries) -version-info 4:0 -no-undefined
libkssl_la_LIBADD = $(LIB_KDEUI) 
libkssl_la_DEPENDENCIES = $(LIB_KDEUI)

libkssl_la_METASOURCES = AUTO

SUBDIRS = kssl

