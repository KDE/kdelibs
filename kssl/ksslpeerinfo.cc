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

#ifdef HAVE_SSL
#include <netdb.h>
#endif

#include "ksslx509map.h"

class KSSLPeerInfoPrivate {
public:
  Q_UINT32 ip;
  QString host;
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

void KSSLPeerInfo::setPeerAddress(QString& addr) {
  d->host = addr;
kdDebug() << "setPeerAddress(" << addr << ")" << endl;
}

void KSSLPeerInfo::setPeerIP(Q_UINT32 ip) {
  d->ip = ip;
kdDebug() << "setPeerIP(" << ip << ")" << endl;
}

bool KSSLPeerInfo::certMatchesAddress() {
#ifdef HAVE_SSL
  KSSLX509Map certinfo(m_cert.getSubject());
  char **addrList;
  struct hostent *he;

  he = gethostbyname(certinfo.getValue("CN").latin1());
  if (!he) return false;

  addrList = he->h_addr_list;
  kdDebug() << "Looking for " << d->ip << endl;
  for (int i = 0; addrList[i]; i++) {
    Q_UINT32 thisAddr;
    thisAddr = (addrList[i][0] << 24)
              +(addrList[i][1] << 16)
              +(addrList[i][2] << 8)
              + addrList[i][3];
    kdDebug() << "       -- found: " << thisAddr << endl;

    if (thisAddr == d->ip) return true;
  }
#endif
  return false;
}
