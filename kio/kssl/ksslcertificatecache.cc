/* This file is part of the KDE project
 *
 * Copyright (C) 2000, 2001 George Staikos <staikos@kde.org>
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
     DCOPCString rettype;
     QDataStream arg(&data, QIODevice::WriteOnly);
     d->dcc->call("kded", "kssld",
                  "cacheReload()",
                  data, rettype, retval);
}


void KSSLCertificateCache::addCertificate(KSSLCertificate& cert, 
                       KSSLCertificatePolicy policy, bool permanent) {
     QByteArray data, retval;
     DCOPCString rettype;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << cert;
     arg << policy;
     arg << permanent;
     d->dcc->call("kded", "kssld",
                  "cacheAddCertificate(KSSLCertificate,KSSLCertificateCache::KSSLCertificatePolicy,bool)",
                  data, rettype, retval);
}


KSSLCertificateCache::KSSLCertificatePolicy KSSLCertificateCache::getPolicyByCN(const QString& cn) {
     QByteArray data, retval;
     DCOPCString rettype;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << cn;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheGetPolicyByCN(QString)",
                            data, rettype, retval);

     if (rc && rettype == "KSSLCertificateCache::KSSLCertificatePolicy") {
        QDataStream retStream(retval);
        KSSLCertificateCache::KSSLCertificatePolicy drc;
        retStream >> drc;
	return drc;
     }
return KSSLCertificateCache::Ambiguous;
}


KSSLCertificateCache::KSSLCertificatePolicy KSSLCertificateCache::getPolicyByCertificate(KSSLCertificate& cert) {
     QByteArray data, retval;
     DCOPCString rettype;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << cert;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheGetPolicyByCertificate(KSSLCertificate)",
                            data, rettype, retval);

     if (rc && rettype == "KSSLCertificateCache::KSSLCertificatePolicy") {
        QDataStream retStream(retval);
        KSSLCertificateCache::KSSLCertificatePolicy drc;
        retStream >> drc;
	return drc;
     }
return KSSLCertificateCache::Ambiguous;
}


bool KSSLCertificateCache::seenCN(const QString& cn) {
     QByteArray data, retval;
     DCOPCString rettype;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << cn;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheSeenCN(QString)",
                            data, rettype, retval);

     if (rc && rettype == "bool") {
        QDataStream retStream(retval);
        bool drc;
        retStream >> drc;
	return drc;
     }

return false;
}


bool KSSLCertificateCache::seenCertificate(KSSLCertificate& cert) {
     QByteArray data, retval;
     DCOPCString rettype;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << cert;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheSeenCertificate(KSSLCertificate)",
                            data, rettype, retval);

     if (rc && rettype == "bool") {
        QDataStream retStream(retval);
        bool drc;
        retStream >> drc;
	return drc;
     }

return false;
}


bool KSSLCertificateCache::isPermanent(KSSLCertificate& cert) {
     QByteArray data, retval;
     DCOPCString rettype;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << cert;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheIsPermanent(KSSLCertificate)",
                            data, rettype, retval);

     if (rc && rettype == "bool") {
        QDataStream retStream(retval);
        bool drc;
        retStream >> drc;
	return drc;
     }

return false;
}


bool KSSLCertificateCache::removeByCN(const QString& cn) {
     QByteArray data, retval;
     DCOPCString rettype;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << cn;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheRemoveByCN(QString)",
                            data, rettype, retval);

     if (rc && rettype == "bool") {
        QDataStream retStream(retval);
        bool drc;
        retStream >> drc;
	return drc;
     }

return false;
}


bool KSSLCertificateCache::removeByCertificate(KSSLCertificate& cert) {
     QByteArray data, retval;
     DCOPCString rettype;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << cert;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheRemoveByCertificate(KSSLCertificate)",
                            data, rettype, retval);

     if (rc && rettype == "bool") {
        QDataStream retStream(retval);
        bool drc;
        retStream >> drc;
	return drc;
     }

return false;
}


bool KSSLCertificateCache::modifyByCN(const QString& cn,
                  KSSLCertificateCache::KSSLCertificatePolicy policy,
                  bool permanent,
                  QDateTime& expires) {
     QByteArray data, retval;
     DCOPCString rettype;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << cn << policy << permanent << expires;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheModifyByCN(QString,KSSLCertificateCache::KSSLCertificatePolicy,bool,QDateTime)",
                            data, rettype, retval);

     if (rc && rettype == "bool") {
        QDataStream retStream(retval);
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
     DCOPCString rettype;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << cert << policy << permanent << expires;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheModifyByCertificate(KSSLCertificate,KSSLCertificateCache::KSSLCertificatePolicy,bool,QDateTime)",
                            data, rettype, retval);

     if (rc && rettype == "bool") {
        QDataStream retStream(retval);
        bool drc;
        retStream >> drc;
        return drc;
     }

return false;
}


QStringList KSSLCertificateCache::getHostList(KSSLCertificate& cert) {
     QByteArray data, retval;
     DCOPCString rettype;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << cert;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheGetHostList(KSSLCertificate)",
                            data, rettype, retval);

     if (rc && rettype == "QStringList") {
        QDataStream retStream(retval);
        QStringList drc;
        retStream >> drc;
	return drc;
     }
return QStringList();
}


bool KSSLCertificateCache::addHost(KSSLCertificate& cert, const QString& host) {
     QByteArray data, retval;
     DCOPCString rettype;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << cert << host;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheAddHost(KSSLCertificate,QString)",
                            data, rettype, retval);

     if (rc && rettype == "bool") {
        QDataStream retStream(retval);
        bool drc;
        retStream >> drc;
        return drc;
     }

return false;
}


bool KSSLCertificateCache::removeHost(KSSLCertificate& cert, const QString& host) {
     QByteArray data, retval;
     DCOPCString rettype;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << cert << host;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheRemoveHost(KSSLCertificate,QString)",
                            data, rettype, retval);

     if (rc && rettype == "bool") {
        QDataStream retStream(retval);
        bool drc;
        retStream >> drc;
        return drc;
     }

return false;
}


QStringList KSSLCertificateCache::getKDEKeyByEmail(const QString &email) {
     QByteArray data, retval;
     DCOPCString rettype;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << email;
     bool rc = d->dcc->call("kded", "kssld",
                            "getKDEKeyByEmail(QString)",
                            data, rettype, retval);

     if (rc && rettype == "QStringList") {
        QDataStream retStream(retval);
        QStringList drc;
        retStream >> drc;
        return drc;
     }

     return QStringList();
}     


KSSLCertificate *KSSLCertificateCache::getCertByMD5Digest(const QString &key) {
     QByteArray data, retval;
     DCOPCString rettype;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << key;
     bool rc = d->dcc->call("kded", "kssld",
                            "getCertByMD5Digest(QString)",
                            data, rettype, retval);

     if (rc && rettype == "KSSLCertificate") {
        QDataStream retStream(retval);
        KSSLCertificate *drc = new KSSLCertificate;
        retStream >> *drc;
	if (drc->getCert())
             return drc; 
	delete drc; // should not happen too often if used in conjunction with getKDEKeyByEmail
     }

     return 0L;
}     


QDataStream& operator<<(QDataStream& s, const KSSLCertificateCache::KSSLCertificatePolicy& p) {
  s << (quint32)p;
return s;
}


QDataStream& operator>>(QDataStream& s, KSSLCertificateCache::KSSLCertificatePolicy& p) {
  quint32 pd;
  s >> pd;
  p = (KSSLCertificateCache::KSSLCertificatePolicy) pd;
  return s;
}





