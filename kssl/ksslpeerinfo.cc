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

#include "ksslx509map.h"

class KSSLPeerInfoPrivate {
public:
  QString host;
};



KSSLPeerInfo::KSSLPeerInfo() {
  d = new KSSLPeerInfoPrivate;
  d->host = "";
}

KSSLPeerInfo::~KSSLPeerInfo() {
  delete d;
}

KSSLCertificate& KSSLPeerInfo::getPeerCertificate() {
  return m_cert;
}

void KSSLPeerInfo::setPeerAddress(QString addr) {
  d->host = addr;
}


bool KSSLPeerInfo::certMatchesAddress() {
#ifdef HAVE_SSL
  KSSLX509Map certinfo(m_cert.getSubject());
  KInetSocketAddress kisa1(d->host, 0, -1);
  KInetSocketAddress kisa2(certinfo.getValue("CN").latin1(), 0, -1);

  kdDebug() << "d->host is: " << kisa1.pretty()
            << " while the CN is: " << kisa2.pretty()
            << endl;
  kdDebug() << "The original ones were: " << d->host
            << " and: " << certinfo.getValue("CN").latin1()
            << endl;

  if (kisa1.isCoreEqual(kisa2)) return true;

#endif
  return false;
}
