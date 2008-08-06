/* This file is part of the KDE project
 *
 * Copyright (C) 2000-2003 George Staikos <staikos@kde.org>
 * Copyright (C) 2000 Malte Starostik <malte@kde.org>
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

#ifndef _KSSLINFODIALOG_H
#define _KSSLINFODIALOG_H

#include <kdialog.h>

#include "ksslx509map.h"
#include "ksslcertificate.h"
#include "kssl.h"
#include <QtNetwork/QSslError>

class QWidget;
class KSSLCertBox;
class QSslCertificate;
class KTcpSocket;

/**
 * KDE SSL Information Dialog
 *
 * This class creates a dialog that can be used to display information about
 * an SSL session.
 *
 * There are NO GUARANTEES that KSSLInfoDialog will remain binary compatible/
 * Contact staikos@kde.org for details if needed.
 *
 * @author George Staikos <staikos@kde.org>
 * @see KSSL
 * @short KDE SSL Information Dialog
 */
class KIO_EXPORT KSSLInfoDialog : public KDialog {
	Q_OBJECT
public:
	/**
	 *  Construct a KSSL Information Dialog
	 *
	 *  @param parent the parent widget
	 */
	explicit KSSLInfoDialog(QWidget *parent = 0);

	/**
	 *  Destroy this dialog
	 */
	virtual ~KSSLInfoDialog();

	/**
	 *  Tell the dialog if the connection has portions that may not be
	 *  secure (ie. a mixture of secure and insecure frames)
	 *
	 *  @param isIt true if security is in question
	 */
	void setSecurityInQuestion(bool isIt);

	/**
	 *  Set information to display about the SSL connection.
	 *
	 *  @param certificateChain the certificate chain leading from the certificate
     *         authority to the peer.
	 *  @param ip the ip of the remote host
	 *  @param url the url being accessed
     *  @param sslProtocol the version of SSL in use (SSLv2, SSLv3, TLSv1)
	 *  @param cipher the cipher in use
	 *  @param usedBits the used bits of the key
	 *  @param bits the key size of the cipher in use
	 *  @param validationErrors errors validating the certificates, if any
	 */
	void setSslInfo(const QList<QSslCertificate> &certificateChain,
			        const QString &ip, const QString &url,
			        const QString &sslProtocol, const QString &cipher,
                    int usedBits, int bits,
			        const QList<QSslError::SslError> &validationErrors);

    void setMainPartEncrypted(bool);
    void setAuxiliaryPartsEncrypted(bool);

private:
    void updateWhichPartsEncrypted();

	class KSSLInfoDialogPrivate;
	KSSLInfoDialogPrivate* const d;

private Q_SLOTS:
	void launchConfig();
	void displayFromChain(int);
};

#endif
