/* This file is part of the KDE project
 *
 * Copyright (C) 2000 George Staikos <staikos@kde.org>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qregexp.h>

#include "ksslpeerinfo.h"
#include <kdebug.h>

#include <ksockaddr.h>
#include <kextsock.h>
#include <netsupp.h>

#include "ksslx509map.h"

class KSSLPeerInfoPrivate {
public:
  KSSLPeerInfoPrivate() {}
  ~KSSLPeerInfoPrivate() { }
  QString peerHost;
};



KSSLPeerInfo::KSSLPeerInfo() {
  d = new KSSLPeerInfoPrivate;
}

KSSLPeerInfo::~KSSLPeerInfo() {
  delete d;
}

KSSLCertificate& KSSLPeerInfo::getPeerCertificate() {
  return m_cert;
}

void KSSLPeerInfo::setPeerHost(QString realHost) {
	d->peerHost = realHost.stripWhiteSpace();
	while(d->peerHost.endsWith("."))
		d->peerHost.truncate(d->peerHost.length()-1);

	d->peerHost = d->peerHost.lower();
}

bool KSSLPeerInfo::certMatchesAddress() {
#ifdef HAVE_SSL
KSSLX509Map certinfo(m_cert.getSubject());
QStringList cns = QStringList::split(QRegExp("[ \n\r]"), certinfo.getValue("CN"));

	for (QStringList::Iterator cn = cns.begin(); cn != cns.end(); ++cn) {
		if (cnMatchesAddress((*cn).stripWhiteSpace().lower()))
			return true;
	}

#endif

return false;
}


bool KSSLPeerInfo::cnMatchesAddress(QString cn) {
#ifdef HAVE_SSL
QRegExp rx;


	kdDebug(7029) << "Matching CN=[" << cn << "] to ["
		      << d->peerHost << "]" << endl;

	// Check for invalid characters
	if (QRegExp("[^a-zA-Z0-9\\.\\*\\-]").search(cn) >= 0) {
		kdDebug(7029) << "CN contains invalid characters!  Failing." << endl;
		return false;
	}

	// Domains can legally end with '.'s.  We don't need them though.
	while(cn.endsWith("."))
		cn.truncate(cn.length()-1);

	// Do not let empty CN's get by!!
	if (cn.isEmpty())
		return false;

	// Check for IPv4 address
	rx.setPattern("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}");
	if (rx.exactMatch(d->peerHost))
		return d->peerHost == cn;

	// Check for IPv6 address here...
	rx.setPattern("^\\[.*\\]$");
	if (rx.exactMatch(d->peerHost))
		return d->peerHost == cn;

	if (cn.contains('*')) {
		// First make sure that there are at least two valid parts
		// after the wildcard (*).
		QStringList parts = QStringList::split('.', cn, false);
    
		while(parts.count() > 2)
			parts.remove(parts.begin());

		if (parts.count() != 2) {
			return false;  // we don't allow *.root - that's bad
		}

		if (parts[0].contains('*') || parts[1].contains('*')) {
			return false;
		}

		// RFC2818 says that *.example.com should match against
		// foo.example.com but not bar.foo.example.com
		// (ie. they must have the same number of parts)
		if (QRegExp(cn, false, true).exactMatch(d->peerHost) &&
		    QStringList::split('.', cn, false).count() == 
		    QStringList::split('.', d->peerHost, false).count())
			return true;

		return false;
	}

	// We must have an exact match in this case (insensitive though)
	// (note we already did .lower())
	if (cn == d->peerHost)
		return true;
#endif
return false;
}


void KSSLPeerInfo::reset() {
   d->peerHost = QString::null;
}


