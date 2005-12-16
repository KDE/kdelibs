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


#ifndef _INCLUDE_KSSLSIGNERS_H
#define _INCLUDE_KSSLSIGNERS_H

class KSSLCertificate;
class DCOPClient;

#include <qstringlist.h>

#include <kdelibs_export.h>

// ### KDE4 - Fix constness
/**
 * KDE SSL Signer Database
 *
 * This class is used to manipulate the KDE SSL signer database.  It
 * communicates to the KDE SSL daemon via dcop for backend integration.
 *
 * @author George Staikos <staikos@kde.org>
 * @see KSSL, KSSLCertificate
 * @short KDE SSL Signer Database
 */
class KIO_EXPORT KSSLSigners {
public:
	/**
	 *  Construct a KSSLSigner object.
	 */
	KSSLSigners();

	/**
	 *  Destroy this KSSLSigner object.
	 */
	~KSSLSigners();

	/**
	 *  Add a signer to the database.
	 *
	 *  @param cert the signer's certificate
	 *  @param ssl allow it to sign for SSL
	 *  @param email allow it to sign for S/MIME
	 *  @param code allow it to sign for code signing
	 *  @return true on success
	 */
	bool addCA(KSSLCertificate& cert, bool ssl, bool email, bool code);

	/**
	 *  Add a signer to the database.
	 *
	 *  @param cert the signer's certificate in base64 form
	 *  @param ssl allow it to sign for SSL
	 *  @param email allow it to sign for S/MIME
	 *  @param code allow it to sign for code signing
	 *  @return true on success
	 */
	bool addCA(const QString &cert, bool ssl, bool email, bool code); 

	/**
	 *  Regenerate the signer-root file from the user's settings.
	 *  @return true on success
	 */
	bool regenerate();

	/**
	 *  Determine if a certificate can be used for SSL certificate signing
	 *  @param cert the certificate
	 *  @return true if it can be used for SSL
	 */
	bool useForSSL(KSSLCertificate& cert);

	/**
	 *  Determine if a certificate can be used for SSL certificate signing
	 *  @param subject the certificate subject
	 *  @return true if it can be used for SSL
	 */
	bool useForSSL( const QString &subject);

	/**
	 *  Determine if a certificate can be used for S/MIME certificate signing
	 *  @param cert the certificate
	 *  @return true if it can be used for S/MIME
	 */
	bool useForEmail(KSSLCertificate& cert);

	/**
	 *  Determine if a certificate can be used for S/MIME certificate signing
	 *  @param subject the certificate subject
	 *  @return true if it can be used for S/MIME
	 */
	bool useForEmail(const QString &subject);

	/**
	 *  Determine if a certificate can be used for code certificate signing
	 *  @param cert the certificate
	 *  @return true if it can be used for code
	 */
	bool useForCode(KSSLCertificate& cert);

	/**
	 *  Determine if a certificate can be used for code certificate signing
	 *  @param subject the certificate subject
	 *  @return true if it can be used for code
	 */
	bool useForCode(const QString &subject);

	/**
	 *  Remove a certificate signer from the database
	 *  @param cert the certificate to remove
	 *  @return true on success
	 */
	bool remove(KSSLCertificate& cert);

	/**
	 *  Remove a certificate signer from the database
	 *  @param subject the subject of the certificate to remove
	 *  @return true on success
	 */
	bool remove(const QString &subject);

	/**
	 *  List the signers in the database.
	 *  @return the list of subjects in the database
	 *  @see getCert
	 */
	QStringList list();

	/**
	 *  Get a signer certificate from the database.
	 *
	 *  @param subject the subject of the certificate desired
	 *  @return the base64 encoded certificate
	 */
	QString getCert(const QString &subject);

	/**
	 *  Set the use of a particular entry in the certificate signer database.
	 *  @param subject the subject of the certificate in question
	 *  @param ssl allow this for SSL certificate signing
	 *  @param email allow this for S/MIME certificate signing
	 *  @param code allow this for code certificate signing
	 *  @return true on success
	 */
	bool setUse(const QString &subject, bool ssl, bool email, bool code);

private:
	class KSSLSignersPrivate;
	KSSLSignersPrivate *d;

	DCOPClient *dcc;
};


#endif
