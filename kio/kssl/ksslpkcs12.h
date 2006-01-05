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

#ifndef _KSSLPKCS12_H
#define _KSSLPKCS12_H

#ifdef Q_WS_WIN
#include "ksslconfig_win.h"
#else
#include "ksslconfig.h"
#endif

#ifdef KSSL_HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/pkcs12.h>
#undef crypt
#else
class PKCS12;
class EVP_PKEY;
class X509;
#endif

#include <ksslcertificate.h>
#include <ksslcertchain.h>

#ifndef STACK_OF
#define STACK_OF(x) void
#endif

class KSSL;
class KSSLPKCS12Private;
class KOpenSSLProxy;

/**
 * KDE PKCS#12 Certificate
 *
 * As of yet, this class is being defined.  if you use it, let it be known
 * that BC will break on you until this message is removed.
 *
 * @author George Staikos <staikos@kde.org>
 * @see KSSL, KSSLCertificate
 * @short KDE PKCS#12 Certificate
 */
class KIO_EXPORT KSSLPKCS12 {
friend class KSSL;

public:
	/**
	 *  Destroy this PKCS#12 certificate
	 */
	virtual ~KSSLPKCS12();

	/**
	 *  The name of this certificate.  This can be used to refer to the
	 *  certificate instead of passing the object itself.
	 *  @return the name of the certificate
	 */
	QString name() const;

	/**
	 *  Create a KSSLPKCS12 object from a Base64 in a QString.
	 *  @param base64 the base64 encoded certificate
	 *  @param password a password for the certificate if encrypted
	 *  @return the PKCS#12 object, or NULL on failure.
	 */
	static KSSLPKCS12* fromString(const QString &base64, const QString &password = "");

	/**
	 *  Create a KSSLPKCS12 object by reading a PKCS#12 file.
	 *  @param filename the filename of the certificate
	 *  @param password a password for the certificate if encrypted
	 *  @return the PKCS#12 object, or NULL on failure.
	 */
	static KSSLPKCS12* loadCertFile(const QString &filename, const QString &password = "");

	/**
	 *  Convert to a Base64 string.
	 *  @return the certificate in base64 form
	 */
	QString toString();

	/**
	 *  Raw set the PKCS12 object.
	 *  @param c the openssl PKCS12 object
	 *  @internal
	 */
	void setCert(PKCS12 *c);

	/**
	 *  Change the password of the PKCS#12 in memory.
	 *  @param pold the old password
	 *  @param pnew the new password
	 *  @return true on success
	 */
	bool changePassword(const  QString &pold, const QString &pnew);
 
	/**
	 *  Get the private key.
	 *  @return the internal OpenSSL private key format
	 *  @internal
	 */
	EVP_PKEY *getPrivateKey();

	/**
	 *  Get the X.509 certificate.
	 *  @return the X.509 certificate for the PKCS#12 object, or NULL
	 */
	KSSLCertificate *getCertificate();

	/**
	 *  Write the PKCS#12 to a file in raw mode.
	 *  @param filename the file to write to
	 *  @return true on success
	 */
	bool toFile(const QString &filename);

	/**
	 *  Check the X.509 and private key to make sure they're valid.
	 *  @return the result of the validation
	 *  @see KSSLCertificate
	 */
	KSSLCertificate::KSSLValidation validate();

	/**
	 *  Check the X.509 and private key to make sure they're valid.
	 *  @param p the purpose to validate for
	 *  @return the result of the validation
	 *  @see KSSLCertificate
	 */
	KSSLCertificate::KSSLValidation validate(KSSLCertificate::KSSLPurpose p);

	/**
	 *  Check the X.509 and private key to make sure they're valid.
	 *  Ignore any cached validation result.
	 *  @return the result of the validation
	 *  @see KSSLCertificate
	 */
	KSSLCertificate::KSSLValidation revalidate();

	/**
	 *  Check the X.509 and private key to make sure they're valid.
	 *  Ignore any cached validation result.
	 *  @param p the purpose to validate for
	 *  @return the result of the validation
	 *  @see KSSLCertificate
	 */
	KSSLCertificate::KSSLValidation revalidate(KSSLCertificate::KSSLPurpose p);

	/**
	 *  Check if the X.509 and private key are valid.
	 *  @return true if valid
	 */
	bool isValid();

	/**
	 *  Check if the X.509 and private key are valid.
	 *  @param p the purpose to validate for
	 *  @return true if valid
	 */
	bool isValid(KSSLCertificate::KSSLPurpose p);

protected:
	KSSLPKCS12();
	bool parse(const QString &pass);

private:
	KSSLPKCS12Private *d;
	PKCS12 *_pkcs;
	KOpenSSLProxy *kossl;
	EVP_PKEY *_pkey;
	KSSLCertificate *_cert;
	STACK_OF(X509) *_caStack;
};


#endif

