/* This file is part of the KDE project
 *
 * Copyright (C) 2003 George Staikos <staikos@kde.org>
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

#include "ksslsession.h"

#include <config.h>
#include <ksslconfig.h>

#include <kopenssl.h>
#include <kcodecs.h>

KSSLSession::KSSLSession() : _session(0L) {
}


KSSLSession::~KSSLSession() {
#ifdef KSSL_HAVE_SSL
	if (_session) {
		KOpenSSLProxy::self()->SSL_SESSION_free(static_cast<SSL_SESSION*>(_session));
		_session = 0L;
	}
#endif
}


QString KSSLSession::toString() const {
    QString rc;
#ifdef KSSL_HAVE_SSL
    SSL_SESSION *session = static_cast<SSL_SESSION*>(_session);
    int slen = KOpenSSLProxy::self()->i2d_SSL_SESSION(session, 0L);

    if (slen >= 0) {
	// These should technically be unsigned char * but it doesn't matter
	// for our purposes
	char *csess = new char[slen];
	unsigned char *p = (unsigned char*)csess;

	if (!KOpenSSLProxy::self()->i2d_SSL_SESSION(session, &p)) {
	    delete[] csess;
	    return QString();
	}

	// encode it into a QString
	rc = QByteArray(csess,slen).toBase64();
	delete[] csess;
     }
#endif
    return rc;
}


KSSLSession *KSSLSession::fromString(const QString& s) {
KSSLSession *session = 0L;
#ifdef KSSL_HAVE_SSL
QByteArray qba, qbb = s.toLocal8Bit();
        qba = QByteArray::fromBase64(qbb);
	unsigned char *qbap = reinterpret_cast<unsigned char *>(qba.data());
	SSL_SESSION *ss = KOSSL::self()->d2i_SSL_SESSION(0L, &qbap, qba.size());
        if (ss) {
		session = new KSSLSession;
		session->_session = ss;
        }
#endif
return session;
}


