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
	d->peerHost = realHost;
}

bool KSSLPeerInfo::certMatchesAddress() {
#ifdef HAVE_SSL
  KSSLX509Map certinfo(m_cert.getSubject());
  QString cn = certinfo.getValue("CN");

    QStringList domains;

    kdDebug(7029) << "Matching CN=" << cn << " to " << d->peerHost << endl;

    extractDomains(d->peerHost, domains);
    QStringList::Iterator it = domains.begin();
    for (; it != domains.end(); it++)
    {
      int match = cn.findRev(*it, -1, false);
      kdDebug(7029) << "Match= " << match << ", CN.length= " << cn.length()
                    << ", host.length= " << (*it).length() << endl;

      if (match > -1 && ((match + (*it).length()) == cn.length()))
      {
        kdDebug(7029) << "Found a match ==> " << (*it) << endl;
        return true;
      }
    }
#endif
    return false;
}

void KSSLPeerInfo::extractDomains(const QString &fqdn, QStringList &domains)
{
    domains.clear();

    // If fqdn is an IP address, then only use
    // the entire IP address to find a match! (DA)
    if (fqdn[0] >= '0' && fqdn[0] <= '9') {
       domains.append(fqdn);
       return;
    }

    QStringList partList = QStringList::split('.', fqdn, false);

    if (partList.count())
        partList.remove(partList.begin()); // Remove hostname

    while(partList.count()) {
       if (partList.count() == 1)
         break; // We only have a TLD left.

       if (partList.count() == 2) {
          // If this is a TLD, we should stop. (e.g. co.uk)
          // We assume this is a TLD if it ends with .xx.yy or .x.yy
          if (partList[0].length() <= 2 && partList[1].length() == 2)
             break; // This is a TLD.
       }

       QString domain = partList.join(".");
       domains.append(domain);
       partList.remove(partList.begin());
    }

    // Add the entire FQDN at the end of the
    // list for fqdn == CN checks
    domains.append(fqdn);
}
