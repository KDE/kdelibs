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


#include "ksslcertificatecache.h"
#include "ksslcertchain.h"
#include "ksslcertificate.h"

#include <stdlib.h>
#include <kdebug.h>
#include <dcopclient.h>
#include <kdatastream.h>


class KSSLCertificateCache::KSSLCertificateCachePrivate {
  public:
  DCOPClient *dcc;

  KSSLCertificateCachePrivate()  { dcc = new DCOPClient; dcc->attach(); }
  ~KSSLCertificateCachePrivate() { delete dcc;}

};



KSSLCertificateCache::KSSLCertificateCache() {
  d = new KSSLCertificateCachePrivate;
}


KSSLCertificateCache::~KSSLCertificateCache() {
  delete d;
}


void KSSLCertificateCache::saveToDisk() {
   kdDebug() << "Deprecated function KSSLCertificateCache::saveToDisk() called" << endl;
}


void KSSLCertificateCache::clearList() {
   kdDebug() << "Deprecated function KSSLCertificateCache::clearList() called" << endl;
}


void KSSLCertificateCache::loadDefaultPolicies() {
   kdDebug() << "Deprecated function KSSLCertificateCache::loadDefaultPolicies() called" << endl;
}


void KSSLCertificateCache::reload() {
     QByteArray data, retval;
     QCString rettype;
     QDataStream arg(data, IO_WriteOnly);
     d->dcc->call("kded", "kssld",
                  "cacheReload()",
                  data, rettype, retval);
}


void KSSLCertificateCache::addCertificate(KSSLCertificate& cert, 
                       KSSLCertificatePolicy policy, bool permanent) {
     QByteArray data, retval;
     QCString rettype;
     QDataStream arg(data, IO_WriteOnly);
     arg << cert;
     arg << policy;
     arg << permanent;
     d->dcc->call("kded", "kssld",
                  "cacheAddCertificate(KSSLCertificate,KSSLCertificateCache::KSSLCertificatePolicy,bool)",
                  data, rettype, retval);
}


KSSLCertificateCache::KSSLCertificatePolicy KSSLCertificateCache::getPolicyByCN(QString& cn) {
     QByteArray data, retval;
     QCString rettype;
     QDataStream arg(data, IO_WriteOnly);
     arg << cn;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheGetPolicyByCN(QString)",
                            data, rettype, retval);

     if (rc && rettype == "KSSLCertificateCache::KSSLCertificatePolicy") {
        QDataStream retStream(retval, IO_ReadOnly);
        KSSLCertificateCache::KSSLCertificatePolicy drc;
        retStream >> drc;
	return drc;
     }
return KSSLCertificateCache::Ambiguous;
}


KSSLCertificateCache::KSSLCertificatePolicy KSSLCertificateCache::getPolicyByCertificate(KSSLCertificate& cert) {
     QByteArray data, retval;
     QCString rettype;
     QDataStream arg(data, IO_WriteOnly);
     arg << cert;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheGetPolicyByCertificate(KSSLCertificate)",
                            data, rettype, retval);

     if (rc && rettype == "KSSLCertificateCache::KSSLCertificatePolicy") {
        QDataStream retStream(retval, IO_ReadOnly);
        KSSLCertificateCache::KSSLCertificatePolicy drc;
        retStream >> drc;
	return drc;
     }
return KSSLCertificateCache::Ambiguous;
}


bool KSSLCertificateCache::seenCN(QString& cn) {
     QByteArray data, retval;
     QCString rettype;
     QDataStream arg(data, IO_WriteOnly);
     arg << cn;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheSeenCN(QString)",
                            data, rettype, retval);

     if (rc && rettype == "bool") {
        QDataStream retStream(retval, IO_ReadOnly);
        bool drc;
        retStream >> drc;
	return drc;
     }

return false;
}


bool KSSLCertificateCache::seenCertificate(KSSLCertificate& cert) {
     QByteArray data, retval;
     QCString rettype;
     QDataStream arg(data, IO_WriteOnly);
     arg << cert;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheSeenCertificate(KSSLCertificate)",
                            data, rettype, retval);

     if (rc && rettype == "bool") {
        QDataStream retStream(retval, IO_ReadOnly);
        bool drc;
        retStream >> drc;
	return drc;
     }

return false;
}


bool KSSLCertificateCache::isPermanent(KSSLCertificate& cert) {
     QByteArray data, retval;
     QCString rettype;
     QDataStream arg(data, IO_WriteOnly);
     arg << cert;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheIsPermanent(KSSLCertificate)",
                            data, rettype, retval);

     if (rc && rettype == "bool") {
        QDataStream retStream(retval, IO_ReadOnly);
        bool drc;
        retStream >> drc;
	return drc;
     }

return false;
}


bool KSSLCertificateCache::removeByCN(QString& cn) {
     QByteArray data, retval;
     QCString rettype;
     QDataStream arg(data, IO_WriteOnly);
     arg << cn;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheRemoveByCN(QString)",
                            data, rettype, retval);

     if (rc && rettype == "bool") {
        QDataStream retStream(retval, IO_ReadOnly);
        bool drc;
        retStream >> drc;
	return drc;
     }

return false;
}


bool KSSLCertificateCache::removeByCertificate(KSSLCertificate& cert) {
     QByteArray data, retval;
     QCString rettype;
     QDataStream arg(data, IO_WriteOnly);
     arg << cert;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheRemoveByCertificate(KSSLCertificate)",
                            data, rettype, retval);

     if (rc && rettype == "bool") {
        QDataStream retStream(retval, IO_ReadOnly);
        bool drc;
        retStream >> drc;
	return drc;
     }

return false;
}


bool KSSLCertificateCache::modifyByCN(QString& cn,
                  KSSLCertificateCache::KSSLCertificatePolicy policy,
                  bool permanent,
                  QDateTime& expires) {
     QByteArray data, retval;
     QCString rettype;
     QDataStream arg(data, IO_WriteOnly);
     arg << cn << policy << permanent << expires;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheModifyByCN(QString,KSSLCertificateCache::KSSLCertificatePolicy,bool,QDateTime)",
                            data, rettype, retval);

     if (rc && rettype == "bool") {
        QDataStream retStream(retval, IO_ReadOnly);
        bool drc;
        retStream >> drc;
	return drc;
     }

return false;
}


bool KSSLCertificateCache::modifyByCertificate(KSSLCertificate& cert,
                           KSSLCertificateCache::KSSLCertificatePolicy policy,
                           bool permanent,
                           QDateTime& expires) {
     QByteArray data, retval;
     QCString rettype;
     QDataStream arg(data, IO_WriteOnly);
     arg << cert << policy << permanent << expires;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheModifyByCertificate(KSSLCertificate,KSSLCertificateCache::KSSLCertificatePolicy,bool,QDateTime)",
                            data, rettype, retval);

     if (rc && rettype == "bool") {
        QDataStream retStream(retval, IO_ReadOnly);
        bool drc;
        retStream >> drc;
	return drc;
     }

return false;
}




QDataStream& operator<<(QDataStream& s, const KSSLCertificateCache::KSSLCertificatePolicy& p) {
  s << (Q_UINT32)p;
return s;
}


QDataStream& operator>>(QDataStream& s, KSSLCertificateCache::KSSLCertificatePolicy& p) {
  Q_UINT32 ui;
  s >> ui;
  p = (KSSLCertificateCache::KSSLCertificatePolicy) ui;
return s;
}



