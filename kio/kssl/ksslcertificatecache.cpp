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
#include <QtDBus/QtDBus>

#include <stdlib.h>
#include <kdebug.h>



KSSLCertificateCache::KSSLCertificateCache() 
	:d(0)
{
}


KSSLCertificateCache::~KSSLCertificateCache() {
   //delete d;
}


void KSSLCertificateCache::saveToDisk() {
   kDebug() << "Deprecated function KSSLCertificateCache::saveToDisk() called" << endl;
}


void KSSLCertificateCache::clearList() {
   kDebug() << "Deprecated function KSSLCertificateCache::clearList() called" << endl;
}


void KSSLCertificateCache::loadDefaultPolicies() {
   kDebug() << "Deprecated function KSSLCertificateCache::loadDefaultPolicies() called" << endl;
}


void KSSLCertificateCache::reload() {
     QDBusInterface("org.kde.kded", "/modules/kssld").call("cacheReload");
}


void KSSLCertificateCache::addCertificate(KSSLCertificate& cert, 
                       KSSLCertificatePolicy policy, bool permanent) {
     QByteArray data;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << cert;
     QDBusInterface("org.kde.kded", "/modules/kssld")
         .call("cacheAddCertificate", data, int(policy), permanent);
}


KSSLCertificateCache::KSSLCertificatePolicy KSSLCertificateCache::getPolicyByCN(const QString& cn) {
     QDBusReply<int> reply = QDBusInterface("org.kde.kded", "/modules/kssld")
                             .call("cacheGetPolicyByCN", cn);
     if (reply.isValid())
        return KSSLCertificatePolicy(reply.value());
return KSSLCertificateCache::Ambiguous;
}


KSSLCertificateCache::KSSLCertificatePolicy KSSLCertificateCache::getPolicyByCertificate(KSSLCertificate& cert) {
     QByteArray data;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << cert;
     QDBusReply<int> reply = QDBusInterface("org.kde.kded", "/modules/kssld")
                             .call("cacheGetPolicyByCertificate", data);

     if (reply.isValid()) 
        return KSSLCertificatePolicy(reply.value());
return KSSLCertificateCache::Ambiguous;
}


bool KSSLCertificateCache::seenCN(const QString& cn) {
     return QDBusReply<bool>(QDBusInterface("org.kde.kded", "/modules/kssld")
                             .call("cacheSeenCN", cn));
}


bool KSSLCertificateCache::seenCertificate(KSSLCertificate& cert) {
     QByteArray data;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << cert;
     return QDBusReply<bool>(QDBusInterface("org.kde.kded", "/modules/kssld")
                             .call("cacheSeenCertificate", data));
}


bool KSSLCertificateCache::isPermanent(KSSLCertificate& cert) {
     QByteArray data, retval;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << cert;
     return QDBusReply<bool>(QDBusInterface("org.kde.kded", "/modules/kssld")
                             .call("cacheIsPermanent", data));
}


bool KSSLCertificateCache::removeByCN(const QString& cn) {
     return QDBusReply<bool>(QDBusInterface("org.kde.kded", "/modules/kssld")
                             .call("cacheRemoveByCN", cn));
}


bool KSSLCertificateCache::removeByCertificate(KSSLCertificate& cert) {
     QByteArray data;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << cert;
     return QDBusReply<bool>(QDBusInterface("org.kde.kded", "/modules/kssld")
                             .call("cacheRemoveByCertificate", data));
}


bool KSSLCertificateCache::modifyByCN(const QString& cn,
                  KSSLCertificateCache::KSSLCertificatePolicy policy,
                  bool permanent,
                  QDateTime& expires) {
     return QDBusReply<bool>(QDBusInterface("org.kde.kded", "/modules/kssld")
                             .call("cacheModifyByCN", cn, int(policy), permanent,
                                   expires.toTime_t()));
}


bool KSSLCertificateCache::modifyByCertificate(KSSLCertificate& cert,
                           KSSLCertificateCache::KSSLCertificatePolicy policy,
                           bool permanent,
                           QDateTime& expires) {
     QByteArray data;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << cert;
     return QDBusReply<bool>(QDBusInterface("org.kde.kded", "/modules/kssld")
                             .call("cacheModifyByCertificate", data, int(policy), permanent,
                                   expires.toTime_t()));
}


QStringList KSSLCertificateCache::getHostList(KSSLCertificate& cert) {
     QByteArray data;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << cert;
     return QDBusReply<QStringList>(QDBusInterface("org.kde.kded", "/modules/kssld")
                                    .call("cacheGetHostList", data));
}


bool KSSLCertificateCache::addHost(KSSLCertificate& cert, const QString& host) {
     QByteArray data;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << cert;
     return QDBusReply<bool>(QDBusInterface("org.kde.kded", "/modules/kssld")
                             .call("cacheAddHost", data, host));
}


bool KSSLCertificateCache::removeHost(KSSLCertificate& cert, const QString& host) {
     QByteArray data;
     QDataStream arg(&data, QIODevice::WriteOnly);
     arg << cert;
     return QDBusReply<bool>(QDBusInterface("org.kde.kded", "/modules/kssld").
                             call("cacheRemoveHost", data, host));
}


QStringList KSSLCertificateCache::getKDEKeyByEmail(const QString &email) {
     return QDBusReply<QStringList>(QDBusInterface("org.kde.kded", "/modules/kssld").
                                    call("getKDEKeyByEmail", email));
}     


KSSLCertificate *KSSLCertificateCache::getCertByMD5Digest(const QString &key) {
     QDBusReply<QByteArray> reply = QDBusInterface("org.kde.kded", "/modules/kssld").
                                    call("getCertByMD5Digest", key);

     if (reply.isValid()) {
        QByteArray data(reply.value());
        QDataStream retStream(data);
        KSSLCertificate *drc = new KSSLCertificate;
        retStream >> *drc;
	if (drc->getCert())
             return drc; 
	delete drc; // should not happen too often if used in conjunction with getKDEKeyByEmail
     }

     return 0L;
}     





