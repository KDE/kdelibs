INCLUDES=$(all_includes) $(SSL_INCLUDES)

noinst_LTLIBRARIES = libkssl.la
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
		ksslpkcs7.h		 \
		ksslpemcallback.h	 \
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
			ksslpkcs7.cc		  \
			ksslpemcallback.cc	  \
			ksslx509map.cc		  \
			keygenwizard.ui		  \
			keygenwizard2.ui

SUBDIRS = kssl

