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

#include "ksslpeerinfo.h"
#include <qstring.h>
#include <kdebug.h>

#include <ksockaddr.h>
#include <kextsock.h>
#include <netsupp.h>

#include "ksslx509map.h"

class KSSLPeerInfoPrivate {
public:
  KSSLPeerInfoPrivate() : host(NULL), proxying(false) {}
  ~KSSLPeerInfoPrivate() { if (host) delete host; }
  KInetSocketAddress *host;
  bool proxying;
  QString proxyHost;
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

void KSSLPeerInfo::setProxying(bool active, QString realHost) {
	d->proxying = active;
	d->proxyHost = realHost;
}

void KSSLPeerInfo::setPeerAddress(KInetSocketAddress& addr) {
  if (!d->host)
    d->host = new KInetSocketAddress(addr);
  else
    (*d->host) = addr;
}


bool KSSLPeerInfo::certMatchesAddress() {
#ifdef HAVE_SSL
  KSSLX509Map certinfo(m_cert.getSubject());
  QString cn = certinfo.getValue("CN");

  if (d->proxying) {
	if (cn.startsWith("*")) {
		QRegExp cnre(cn.lower(), false, true);
		if (cnre.match(d->proxyHost.lower()) >= 0) return true;
	} else {
		if (cn.lower() == d->proxyHost.lower()) return true;
	}
	return false;
  }


  if (cn.startsWith("*")) {   // stupid wildcard cn
     QRegExp cnre(cn.lower(), false, true);
     QString host, port;

     if (KExtendedSocket::resolve(d->host, host, port, NI_NAMEREQD) != 0) 
        host = d->host->nodeName();

     kdDebug(7029) << "Matching CN=" << cn << " to " << host << endl;
     if (cnre.match(host.lower()) >= 0) return true;
  } else {
     int err = 0;
     QList<KAddressInfo> cns = KExtendedSocket::lookup(cn.latin1(), 0, 0, &err);
     if (err != 0) {
       kdDebug(7029) << "Address lookup failed! -- " << err << endl;
       return false;
     }
     cns.setAutoDelete(true);

//     kdDebug(7029) << "The original ones were: " << d->host->nodeName()
//                   << " and: " << certinfo.getValue("CN").latin1()
//                   << endl;

     for (KAddressInfo *x = cns.first(); x; x = cns.next()) {
        if ((*x).address()->isCoreEqual(d->host)) {
           return true;
        }
     }
  }

#endif
  return false;
}
