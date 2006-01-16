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

#ifndef _KSSLCERTIFICATEHOME_H
#define _KSSLCERTIFICATEHOME_H

class KSSLCertificate;
class KSSLPKCS12;
#include <qstring.h>
#include <qstringlist.h>

#include <kdelibs_export.h>

class KIO_EXPORT KSSLCertificateHome {
 
public:

	// AuthNone means there is no policy.  AuthDont means _don't_ _send_!!
	enum KSSLAuthAction {AuthNone, AuthSend, AuthPrompt, AuthDont};
	/*
	 *  These methods might dynamically allocate an object for you.  Be sure
	 *  to delete them when you are done.
	 */
	static KSSLPKCS12* getCertificateByHost(const QString &host, const QString &password, KSSLAuthAction* aa);
	static KSSLPKCS12* getCertificateByName(const QString &name, const QString &password);
	static KSSLPKCS12* getCertificateByName(const QString &name);
	static QString getDefaultCertificateName(const QString &host, KSSLAuthAction *aa = NULL);
	static QString getDefaultCertificateName(KSSLAuthAction *aa = NULL);
	static KSSLPKCS12* getDefaultCertificate(const QString &password, KSSLAuthAction *aa = NULL);
	static KSSLPKCS12* getDefaultCertificate(KSSLAuthAction *aa = NULL);
	static bool hasCertificateByName(const QString &name);


	/*
	 *   These set the default certificate for hosts without a policy.
	 */
	static void setDefaultCertificate(const QString &name, bool send = true, bool prompt = false);
	static void setDefaultCertificate(KSSLPKCS12 *cert, bool send = true, bool prompt = false);


	/*
	 *   These set the default certificate for a host.
	 */
	static void setDefaultCertificate(const QString &name, const QString &host, bool send = true, bool prompt = false);
	static void setDefaultCertificate(KSSLPKCS12 *cert, const QString &host, bool send = true, bool prompt = false);

	/*
	 *   These add a certificate to the repository.
	 *   Returns: true on success, false error
	 */
	static bool addCertificate(const QString &filename, const QString &password, bool storePass = false);
	static bool addCertificate(KSSLPKCS12 *cert, const QString &passToStore = QString());

	/*
	 *   These deletes a certificate from the repository.
	 *   Returns: true on success, false error
	 */
	static bool deleteCertificate(const QString &filename, const QString &password);
	static bool deleteCertificate(KSSLPKCS12 *cert);
	static bool deleteCertificateByName(const QString &name);
 
	/*
	 *   Returns the list of certificates available
	 */
	static QStringList getCertificateList();

private:
	class KSSLCertificateHomePrivate;
	KSSLCertificateHomePrivate* d;
};

#endif

