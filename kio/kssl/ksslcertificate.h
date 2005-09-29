/* This file is part of the KDE project
 *
 * Copyright (C) 2000-2003 George Staikos <staikos@kde.org>
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

#ifndef _KSSLCERTIFICATE_H
#define _KSSLCERTIFICATE_H


// UPDATE: I like the structure of this class less and less every time I look
//         at it.  I think it needs to change.
//
//
//  The biggest reason for making everything protected here is so that
//  the class can have all it's methods available even if openssl is not
//  available.  Also, to create a new certificate you should use the
//  KSSLCertificateFactory, and to manage the user's database of certificates,
//  you should go through the KSSLCertificateHome.
//
//  There should be no reason to touch the X509 stuff directly.
//

#include <qcstring.h>
#include <qvaluelist.h>

class QString;
class QStringList;
class QCString;
class KSSL;
class KSSLCertificatePrivate;
class QDateTime;
class KSSLCertChain;
class KSSLX509V3;

#include <kdelibs_export.h>

#ifdef Q_WS_WIN
#include "ksslconfig_win.h"
#else
#include "ksslconfig.h"
#endif

#ifdef KSSL_HAVE_SSL
typedef struct x509_st X509;
#else
class X509;
#endif

/**
 * KDE X.509 Certificate
 *
 * This class represents an X.509 (SSL) certificate.
 * Note: this object is VERY HEAVY TO COPY.  Please try to use reference
 *       or pointer whenever possible
 *
 * @author George Staikos <staikos@kde.org>
 * @see KSSL
 * @short KDE X.509 Certificate
 */
class KIO_EXPORT KSSLCertificate {
friend class KSSL;
friend class KSSLCertificateHome;
friend class KSSLCertificateFactory;
friend class KSSLCertificateCache;
friend class KSSLCertChain;
friend class KSSLPeerInfo;
friend class KSSLPKCS12;
friend class KSSLD;
friend class KSMIMECryptoPrivate;


public:
	/**
	 *  Destroy this X.509 certificate.
	 */
	~KSSLCertificate();

	/**
	 *  Create an X.509 certificate from a base64 encoded string.
	 *  @param cert the certificate in base64 form
	 *  @return the X.509 certificate, or NULL
	 */
	static KSSLCertificate *fromString(QCString cert);

	/**
	 *  Create an X.509 certificate from the internal representation.
	 *  This one duplicates the X509 object for itself.
	 *  @param x5 the OpenSSL representation of the certificate
	 *  @return the X.509 certificate, or NULL
	 *  @internal
	 */
	static KSSLCertificate *fromX509(X509 *x5);

        /**
         * A CA certificate can be validated as Irrelevant when it was
         * not used to sign any other relevant certificate.
         */
	enum KSSLValidation {   Unknown, Ok, NoCARoot, InvalidPurpose,
				PathLengthExceeded, InvalidCA, Expired,
				SelfSigned, ErrorReadingRoot, NoSSL,
				Revoked, Untrusted, SignatureFailed,
				Rejected, PrivateKeyFailed, InvalidHost,
				Irrelevant, SelfSignedChain
				};

	enum KSSLPurpose {      None=0, SSLServer=1, SSLClient=2,
				SMIMESign=3, SMIMEEncrypt=4, Any=5 };

        typedef QValueList<KSSLValidation> KSSLValidationList;

	/**
	 *  Convert this certificate to a string.
	 *  @return the certificate in base64 format
	 */
	QString toString();

	/**
	 *  Get the subject of the certificate (X.509 map).
	 *  @return the subject
	 */
	QString getSubject() const;

	/**
	 *  Get the issuer of the certificate (X.509 map).
	 *  @return the issuer
	 */
	QString getIssuer() const;

	/**
	 *  Get the date that the certificate becomes valid on.
	 *  @return the date as a string, localised
	 */
	QString getNotBefore() const;

	/**
	 *  Get the date that the certificate is valid until.
	 *  @return the date as a string, localised
	 */
	QString getNotAfter() const;

	/**
	 *  Get the date that the certificate becomes valid on.
	 *  @return the date
	 */
	QDateTime getQDTNotBefore() const;

	/**
	 *  Get the date that the certificate is valid until.
	 *  @return the date
	 */
	QDateTime getQDTNotAfter() const;

	/**
	 *  Convert the certificate to DER (ASN.1) format.
	 *  @return the binary data of the DER encoding
	 */
	QByteArray toDer();

	/**
	 *  Convert the certificate to PEM (base64) format.
	 *  @return the binary data of the PEM encoding
	 */
	QByteArray toPem();

	/**
	 *  Convert the certificate to Netscape format.
	 *  @return the binary data of the Netscape encoding
	 */
	QByteArray toNetscape();

	/**
	 *  Convert the certificate to OpenSSL plain text format.
	 *  @return the OpenSSL text encoding
	 */
	QString toText();

	/**
	 *  Get the serial number of the certificate.
	 *  @return the serial number as a string
	 */
	QString getSerialNumber() const;

	/**
	 *  Get the key type (RSA, DSA, etc).
	 *  @return the key type as a string
	 */
	QString getKeyType() const;

	/**
	 *  Get the public key.
	 *  @return the public key as a hexidecimal string
	 */
	QString getPublicKeyText() const;

	/**
	 *  Get the MD5 digest of the certificate.
	 *  Result is padded with : to separate bytes - it's a text version!
	 *  @return the MD5 digest in a hexidecimal string
	 */
	QString getMD5DigestText() const;

	/**
	 *  Get the MD5 digest of the certificate.
	 *  @return the MD5 digest in a hexidecimal string
	 */
	QString getMD5Digest() const;

	/**
	 *  Get the signature.
	 *  @return the signature in text format
	 */
	QString getSignatureText() const;

	/**
	 *  Check if this is a valid certificate.  Will use cached data.
	 *  @return true if it is valid
	 */
	bool isValid();

	/**
	 *  Check if this is a valid certificate.  Will use cached data.
	 *  @param p the purpose to validate for
	 *  @return true if it is valid
	 */
	bool isValid(KSSLPurpose p);

	/**
	 *  The alternate subject name.
	 *  @return string list with subjectAltName
	 */
	QStringList subjAltNames() const;

	/**
	 *  Check if this is a valid certificate.  Will use cached data.
	 *  @return the result of the validation
	 */
	KSSLValidation validate();

	/**
	 *  Check if this is a valid certificate.  Will use cached data.
	 *  @param p the purpose to validate for
	 *  @return the result of the validation
	 */
	KSSLValidation validate(KSSLPurpose p);

	/**
	 *  Check if this is a valid certificate.  Will use cached data.
	 *  @param p the purpose to validate for
	 *  @return all problems encountered during validation
	 */
	KSSLValidationList validateVerbose(KSSLPurpose p);

	/**
	 *  Check if the certificate ca is a proper CA for this
	 *  certificate.
	 *  @param p the purpose to validate for
	 *  @param ca the certificate to check
	 *  @return all problems encountered during validation
	 */
	KSSLValidationList validateVerbose(KSSLPurpose p, KSSLCertificate *ca);

	/**
	 *  Check if this is a valid certificate.  Will NOT use cached data.
	 *  @return the result of the validation
	 */
	KSSLValidation revalidate();

	/**
	 *  Check if this is a valid certificate.  Will NOT use cached data.
	 *  @param p the purpose to validate for
	 *  @return the result of the validation
	 */
	KSSLValidation revalidate(KSSLPurpose p);

	/**
	 *  Get a reference to the certificate chain.
	 *  @return reference to the chain
	 */
	KSSLCertChain& chain();

	/**
	 *  Obtain the localized message that corresponds to a validation result.
	 *  @param x the code to look up
	 *  @return the message text corresponding to the validation code
	 */
	static QString verifyText(KSSLValidation x);

	/**
	 *  Explicitly make a copy of this certificate.
	 *  @return a copy of the certificate
	 */
	KSSLCertificate *replicate();

	/**
	 *  Copy constructor.  Beware, this is very expensive.
	 *  @param x the object to copy from
	 */
	KSSLCertificate(const KSSLCertificate& x); // copy constructor

	/**
	 *  Re-set the certificate from a base64 string.
	 *  @param cert the certificate to set to
	 *  @return true on success
	 */
	bool setCert(QString& cert);

	/**
	 *  Access the X.509v3 parameters.
	 *  @return reference to the extension object
	 *  @see KSSLX509V3
	 */
	KSSLX509V3& x509V3Extensions();

	/**
	 *  Check if this is a signer certificate.
	 *  @return true if this is a signer certificate
	 */
	bool isSigner();

	/**
	 *  FIXME: document
	 */
	void getEmails(QStringList& to) const;

	/**
	 * KDEKey is a concatenation "Subject (MD5)", mostly needed for SMIME.
	 * The result of getKDEKey might change and should not be used for
	 * persistant storage.
	 */
	QString getKDEKey() const;

	/**
	 * Aegypten semantics force us to search by MD5Digest only.
	 */
	static QString getMD5DigestFromKDEKey(const QString& k);

private:
	KIO_EXPORT friend int operator!=(KSSLCertificate& x, KSSLCertificate& y);
	KIO_EXPORT friend int operator==(KSSLCertificate& x, KSSLCertificate& y);

	KSSLCertificatePrivate *d;
	int purposeToOpenSSL(KSSLPurpose p) const;

protected:
	KSSLCertificate();

	void setCert(X509 *c);
	void setChain(void *c);
	X509 *getCert();
	KSSLValidation processError(int ec);
};

KIO_EXPORT QDataStream& operator<<(QDataStream& s, const KSSLCertificate& r);
KIO_EXPORT QDataStream& operator>>(QDataStream& s, KSSLCertificate& r);

KIO_EXPORT int operator==(KSSLCertificate& x, KSSLCertificate& y);
KIO_EXPORT inline int operator!=(KSSLCertificate& x, KSSLCertificate& y)
{ return !(x == y); }

#endif

