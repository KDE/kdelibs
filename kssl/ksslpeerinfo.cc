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
    QStringList domains;

    kdDebug(7029) << "Matching CN=" << cn << " to " << d->proxyHost << endl;

    extractDomains(d->proxyHost, domains);
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
    return false;
  }


  if (cn.startsWith("*")) {   // stupid wildcard cn
     QString host, port;
     QStringList domains;

     if (KExtendedSocket::resolve(d->host, host, port, NI_NAMEREQD) != 0)
        host = d->host->nodeName();

     kdDebug(7029) << "Matching CN=" << cn << " to " << host << endl;

     extractDomains( host, domains );
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

     return false;
  } else {
     int err = 0;
#if QT_VERSION < 300
     QList<KAddressInfo> cns = KExtendedSocket::lookup(cn.latin1(), 0, 0, &err);
#else
     QPtrList<KAddressInfo> cns = KExtendedSocket::lookup(cn.latin1(), 0, 0, &err);
#endif
     cns.setAutoDelete(true);


     kdDebug(7029) << "The original ones were: " << d->host->nodeName()
                   << " and: " << certinfo.getValue("CN").latin1()
                   << endl;

     for (KAddressInfo *x = cns.first(); x; x = cns.next()) {
        if ((*x).address()->isCoreEqual(d->host)) {
           return true;
        }
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
