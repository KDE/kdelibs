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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "ksslx509v3.h"
#include <config.h>
#include <kopenssl.h>

#ifdef HAVE_SSL
#ifndef V1_ROOT
#define V1_ROOT (EXFLAG_V1|EXFLAG_SS)
#endif
#endif


KSSLX509V3::KSSLX509V3() {
	_flags = 0;
	_keyUsage = 0;
	_xKeyUsage = 0;
	_nsCert = 0;
}


KSSLX509V3::~KSSLX509V3() {

}


bool KSSLX509V3::trustCompatible() {
#ifdef HAVE_SSL
	return (_flags & EXFLAG_SS);
#endif
	return false;
}


/* When reading this, please remember that
 * !A || B    is logically equivalent to   A => B
 */

bool KSSLX509V3::certTypeCA() {
#ifdef HAVE_SSL
	return (((_flags & EXFLAG_CA) && (_flags & EXFLAG_BCONS)) ||
		((_flags & V1_ROOT) == V1_ROOT) ||
		(_flags & EXFLAG_KUSAGE) ||
		(!(_flags & EXFLAG_BCONS) && 
		  (_flags & EXFLAG_NSCERT) && 
		  (_flags & EXFLAG_SS) /*&& 
		  (_nsCert & (NS_OBJSIGN_CA|NS_SMIME_CA|NS_SSL_CA))*/
		)
	       ) 
		? true : false;
#endif
	return false;
}


bool KSSLX509V3::certTypeSSLCA() {
#ifdef HAVE_SSL
	return (certTypeCA() &&
		(!(_flags & EXFLAG_NSCERT) || _nsCert & (NS_SSL_CA|NS_SSL_CLIENT|NS_SSL_SERVER)) &&
		(!(_flags & EXFLAG_XKUSAGE) || _xKeyUsage == 0 || _xKeyUsage & (XKU_SSL_SERVER|XKU_SSL_CLIENT))) ? true : false;
#endif
	return false;
}


bool KSSLX509V3::certTypeEmailCA() {
#ifdef HAVE_SSL
	return (certTypeCA() &&
		(!(_flags & EXFLAG_NSCERT) || _nsCert & (NS_SMIME_CA|NS_SMIME)) &&
		(!(_flags & EXFLAG_XKUSAGE) || _xKeyUsage == 0 || _xKeyUsage & XKU_SMIME)) ? true : false;
#endif
	return false;
}


bool KSSLX509V3::certTypeCodeCA() {
#ifdef HAVE_SSL
	return (certTypeCA() &&
		(!(_flags & EXFLAG_NSCERT) || _nsCert & (NS_OBJSIGN_CA|NS_OBJSIGN)) &&
		(!(_flags & EXFLAG_XKUSAGE) || _xKeyUsage == 0 || _xKeyUsage & XKU_CODE_SIGN)) ? true : false;
#endif
	return false;
}


bool KSSLX509V3::certTypeSSLClient() {
#ifdef HAVE_SSL
	return ((!(_flags & EXFLAG_XKUSAGE) || (_xKeyUsage & XKU_SSL_CLIENT)) ||
		((!certTypeCA() || certTypeSSLCA()) &&
		 (!(_flags & EXFLAG_KUSAGE) || (_keyUsage & X509v3_KU_DIGITAL_SIGNATURE)) &&
		 (!(_flags & EXFLAG_NSCERT) || (_nsCert & NS_SSL_CLIENT))));
#endif
	return false;
}


bool KSSLX509V3::certTypeSSLServer() {
#ifdef HAVE_SSL
	return ((!(_flags & EXFLAG_XKUSAGE) || (_xKeyUsage & (XKU_SSL_SERVER|XKU_SGC))) ||
		((!certTypeCA() || certTypeSSLCA()) &&
		 (!(_flags & EXFLAG_NSCERT) || (_nsCert & NS_SSL_SERVER)) &&
		 (!(_flags & EXFLAG_KUSAGE) || (_keyUsage & (X509v3_KU_DIGITAL_SIGNATURE|X509v3_KU_KEY_ENCIPHERMENT)))
		));
#endif
	return false;
}


bool KSSLX509V3::certTypeNSSSLServer() {
#ifdef HAVE_SSL
	return (certTypeSSLServer() &&
		!certTypeCA() &&
		(!(_flags & EXFLAG_KUSAGE) || (_keyUsage & X509v3_KU_KEY_ENCIPHERMENT)));
#endif
	return false;
}


bool KSSLX509V3::certTypeSMIME() {
#ifdef HAVE_SSL
	return ((!(_flags & EXFLAG_XKUSAGE) || (_xKeyUsage & XKU_SMIME)) &&
		(!certTypeCA() || 
		 (!(_flags & EXFLAG_NSCERT) || (_nsCert & NS_SMIME_CA))) &&
		(!(_flags & EXFLAG_NSCERT) || (_nsCert & (NS_SMIME|NS_SSL_CLIENT))));
#endif
	return false;
}


bool KSSLX509V3::certTypeSMIMEEncrypt() {
#ifdef HAVE_SSL
	return (!certTypeCA() && 
		certTypeSMIME() &&
		(!(_flags & EXFLAG_KUSAGE) || (_keyUsage & X509v3_KU_KEY_ENCIPHERMENT)));
#endif
	return false;
}


bool KSSLX509V3::certTypeSMIMESign() {
#ifdef HAVE_SSL
	return (!certTypeCA() && 
		certTypeSMIME() &&
		(!(_flags & EXFLAG_KUSAGE) || (_keyUsage & (X509v3_KU_DIGITAL_SIGNATURE|X509v3_KU_NON_REPUDIATION))));
#endif
	return false;
}


bool KSSLX509V3::certTypeCRLSign() {
#ifdef HAVE_SSL
	return (certTypeCA() ||
		(!(_flags & EXFLAG_KUSAGE) || (_keyUsage & X509v3_KU_CRL_SIGN)));
#endif
	return false;
}




void KSSLX509V3::setFlags(unsigned long flags,
		          unsigned long keyUsage,
		          unsigned long extendedKeyUsage,
		          unsigned long nsCert) {
	_flags = flags;
	_keyUsage = keyUsage;
	_xKeyUsage = extendedKeyUsage;
	_nsCert = nsCert;
}



