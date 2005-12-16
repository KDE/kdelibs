/* This file is part of the KDE project
 *
 * Copyright (C) 2001 George Staikos <staikos@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ksslx509v3.h"
#include <kopenssl.h>
#include <kdebug.h>


KSSLX509V3::KSSLX509V3() {
	flags = 0;
}


KSSLX509V3::~KSSLX509V3() {
}


/* When reading this, please remember that
 * !A || B    is logically equivalent to   A => B
 */

bool KSSLX509V3::certTypeCA() const {
#ifdef KSSL_HAVE_SSL
	// First try CA without X509_PURPOSE_ANY CA, then just try SSLCA
	return (flags & (65471L << 16)) ? true : certTypeSSLCA();
#endif
	return false;
}


bool KSSLX509V3::certTypeSSLCA()  const {
#ifdef KSSL_HAVE_SSL
	return (flags & ((1 << (16+X509_PURPOSE_NS_SSL_SERVER-1))|
                         (1 << (16+X509_PURPOSE_SSL_SERVER-1))|
                         (1 << (16+X509_PURPOSE_SSL_CLIENT-1)))) ? true : 
		(false || ((1 << (16+X509_PURPOSE_ANY-1)) &&
			   (certTypeSSLServer() ||
			    certTypeSSLClient() ||
			    certTypeNSSSLServer())));
#endif
	return false;
}


bool KSSLX509V3::certTypeEmailCA() const {
#ifdef KSSL_HAVE_SSL
	return (flags & ((1 << (16+X509_PURPOSE_SMIME_ENCRYPT-1))|
                         (1 << (16+X509_PURPOSE_SMIME_SIGN-1)))) ? true : 
		(false || ((1 << (16+X509_PURPOSE_ANY-1)) &&
			   certTypeSMIME()));
#endif
	return false;
}


bool KSSLX509V3::certTypeCodeCA() const {
#ifdef KSSL_HAVE_SSL
	return (flags & (1 << (16+X509_PURPOSE_ANY-1))) ? true : false;
#endif
	return false;
}


bool KSSLX509V3::certTypeSSLClient() const {
#ifdef KSSL_HAVE_SSL
	return (flags & (1 << (X509_PURPOSE_SSL_CLIENT-1))) ? true : false;
#endif
	return false;
}


bool KSSLX509V3::certTypeSSLServer() const {
#ifdef KSSL_HAVE_SSL
	return (flags & (1 << (X509_PURPOSE_SSL_SERVER-1))) ? true : false;
#endif
	return false;
}


bool KSSLX509V3::certTypeNSSSLServer() const {
#ifdef KSSL_HAVE_SSL
	return (flags & (1 << (X509_PURPOSE_NS_SSL_SERVER-1))) ? true : false;
#endif
	return false;
}


bool KSSLX509V3::certTypeSMIME() const {
#ifdef KSSL_HAVE_SSL
	return certTypeSMIMEEncrypt()||certTypeSMIMESign();
#endif
	return false;
}


bool KSSLX509V3::certTypeSMIMEEncrypt() const {
#ifdef KSSL_HAVE_SSL
	return (flags & (1 << (X509_PURPOSE_SMIME_ENCRYPT-1))) ? true : false;
#endif
	return false;
}


bool KSSLX509V3::certTypeSMIMESign() const {
#ifdef KSSL_HAVE_SSL
	return (flags & (1 << (X509_PURPOSE_SMIME_SIGN-1))) ? true : false;
#endif
	return false;
}


bool KSSLX509V3::certTypeCRLSign() const {
#ifdef KSSL_HAVE_SSL
	return (flags & (1 << (X509_PURPOSE_CRL_SIGN-1))) ? true : false;
#endif
	return false;
}





