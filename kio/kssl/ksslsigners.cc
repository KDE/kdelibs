/* This file is part of the KDE project
 *
 * Copyright (C) 2001 George Staikos <staikos@kde.org>
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

#include "ksslsigners.h"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtDBus/QtDBus>
#include "ksslcertificate.h"
#include <stdlib.h>
#include <kdebug.h>


KSSLSigners::KSSLSigners() {
}


KSSLSigners::~KSSLSigners() {
}

bool KSSLSigners::addCA(KSSLCertificate& cert,
                        bool ssl,
                        bool email,
                        bool code) const {
	return addCA(cert.toString(), ssl, email, code);
}


bool KSSLSigners::addCA(const QString &cert,
                        bool ssl,
                        bool email,
                        bool code) const {
     return QDBusReply<bool>(QDBusInterface("org.kde.kded", "/modules/kssld").
                             call("caAdd", cert, ssl, email, code));
}


bool KSSLSigners::regenerate() {
     return QDBusReply<bool>(QDBusInterface("org.kde.kded", "/modules/kssld").
                             call("caRegenerate"));
}


bool KSSLSigners::useForSSL(KSSLCertificate& cert) const {
	return useForSSL(cert.getSubject());
}


bool KSSLSigners::useForSSL(const QString &subject) const{
     return QDBusReply<bool>(QDBusInterface("org.kde.kded", "/modules/kssld").
                             call("caUseForSSL", subject));
}


bool KSSLSigners::useForEmail(KSSLCertificate& cert) const{
	return useForEmail(cert.getSubject());
}


bool KSSLSigners::useForEmail(const QString &subject) const{
     return QDBusReply<bool>(QDBusInterface("org.kde.kded", "/modules/kssld").
                             call("caUseForEmail", subject));
}


bool KSSLSigners::useForCode(KSSLCertificate& cert) const{
	return useForCode(cert.getSubject());
}


bool KSSLSigners::useForCode(const QString &subject) const{
     return QDBusReply<bool>(QDBusInterface("org.kde.kded", "/modules/kssld").
                             call("caUseForCode", subject));
}


bool KSSLSigners::remove(KSSLCertificate& cert) {
	return remove(cert.getSubject());
}


bool KSSLSigners::remove(const QString &subject) {
     return QDBusReply<bool>(QDBusInterface("org.kde.kded", "/modules/kssld").
                             call("caRemove", subject));
}


QStringList KSSLSigners::list() {
     return QDBusReply<QStringList>(QDBusInterface("org.kde.kded", "/modules/kssld").
                                    call("caList"));
}


QString KSSLSigners::getCert(const QString &subject) {
     return QDBusReply<QString>(QDBusInterface("org.kde.kded", "/modules/kssld").
                                call("caGetCert", subject));
}


bool KSSLSigners::setUse(const QString &subject, bool ssl, bool email, bool code) {
     return QDBusReply<bool>(QDBusInterface("org.kde.kded", "/modules/kssld").
                             call("caSetUse", subject, ssl, email, code));
}




