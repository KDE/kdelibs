/* This file is part of the KDE project
 *
 * Copyright (C) 2001-2003 George Staikos <staikos@kde.org>
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

#ifndef _KSSLPKCS7_H
#define _KSSLPKCS7_H

#ifdef Q_WS_WIN
#include "ksslconfig_win.h"
#else
#include "ksslconfig.h"
#endif

#ifdef KSSL_HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/pkcs7.h>
#undef crypt
#else
class PKCS7;
class X509;
#endif

//#include <kopenssl.h>
#include <ksslcertificate.h>
#include <ksslcertchain.h>

#ifndef STACK_OF
#define STACK_OF(x) void
#endif

class KSSL;
class KSSLPKCS7Private;
class KOpenSSLProxy;

/**
 * KDE PKCS#7 Certificate
 *
 * This class represents a PKCS#7 certificate
 *
 * @author George Staikos <staikos@kde.org>
 * @see KSSL
 * @short KDE PKCS#7 Certificate
 */
class KIO_EXPORT KSSLPKCS7 {
friend class KSSL;

public:
	/**
	 *  Destroy this PKCS#7 certificate
	 */
	virtual ~KSSLPKCS7();

	/**
	 *  The name of this certificate.  This can be used to refer to the
	 *  certificate instead of passing the object itself.
	 *  @return the name of the certificate
	 */
	QString name() const;

	/**
	 *  Create a KSSLPKCS7 object from a Base64 in a QString.
	 *  @param base64 the base64 representation of the certificate
	 *  @return a PKCS#7 object, or NULL on failure
	 */
	static KSSLPKCS7* fromString(const QString &base64);

	/**
	 *  Create a KSSLPKCS7 object by reading a PKCS#7 file.
	 *  @param filename the filename to read the certificate from
	 *  @return a PKCS#7 object, or NULL on failure
	 */
	static KSSLPKCS7* loadCertFile(const QString &filename);

	/**
	 *  Convert to a Base64 string.
	 *  @return the PKCS#7 object in base64 form
	 */
	QString toString() const;

	/**
	 *  Raw set the PKCS7 object.
	 *  @param c the PKCS7 object
	 *  @internal
	 */
	void setCert(PKCS7 *c);

	/**
	 *  Get the bottom level X.509 certificate.
	 *  @return the certificate, or NULL on failure
	 *  @see KSSLCertificate
	 */
	KSSLCertificate *getCertificate();

	/**
	 *  Get the certificate chain.
	 *  @return the certificate chain
	 *  @see KSSLCertChain
	 */
	KSSLCertChain *getChain();

	/**
	 *  Write the PKCS#7 to a file in raw mode.
	 *  @param filename the filename to write
	 *  @return true on success
	 */
	bool toFile(const QString &filename);

	/**
	 *  Check the chain to make sure it's valid.
	 *  @return the result of the validation procedure
	 */
	KSSLCertificate::KSSLValidation validate() const;

	/**
	 *  Check the chain to make sure it's valid.
	 *  Ignore any cached validation result.
	 *  @return the result of the validation
	 *  @see KSSLCertificate
	 */
	KSSLCertificate::KSSLValidation revalidate();

	/**
	 *  Return true if the chain is valid.
	 */
	bool isValid() const;

protected:
	KSSLPKCS7();

private:
	KSSLPKCS7Private *d;
	PKCS7 *_pkcs;
	KOpenSSLProxy *kossl;
	KSSLCertificate *_cert;
	KSSLCertChain *_chain;
};


#endif

