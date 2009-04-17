/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2009 Michael Leupold <lemma@confuego.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include <kio/authinfo.h>
#include <QtCore/QByteArray>
#include <QtCore/QEventLoop>
#include <kdebug.h>

#include "kpasswdserver_p.h"
#include "kpasswdserverloop_p.h"
#include "kpasswdserver_interface.h"

namespace KIO
{

KPasswdServer::KPasswdServer()
    : m_interface(new OrgKdeKPasswdServerInterface("org.kde.kded",
                                                   "/modules/kpasswdserver",
                                                   QDBusConnection::sessionBus()))
{
}

KPasswdServer::~KPasswdServer()
{
    delete m_interface;
}

qlonglong KPasswdServer::checkAuthInfo(KIO::AuthInfo &info, qlonglong windowId,
                                       qlonglong usertime)
{
    kDebug(7019) << "window-id=" << windowId << "url=" << info.url;

    // create the loop for waiting for a result before sending the request
    KPasswdServerLoop loop;
    connect(m_interface,
            SIGNAL(checkAuthInfoAsyncResult(qlonglong, qlonglong, const KIO::AuthInfo &)),
            &loop, SLOT(slotQueryResult(qlonglong, qlonglong, const KIO::AuthInfo &)));
            
    QDBusReply<qlonglong> reply = m_interface->checkAuthInfoAsync(info, windowId,
                                                                  usertime);
    if (!reply.isValid()) {
        if (reply.error().type() == QDBusError::UnknownMethod) {
            qlonglong res = legacyCheckAuthInfo(info, windowId, usertime);
            if (res > 0) {
                return res;
            }
        }

        kWarning(7019) << "Can't communicate with kded_kpasswdserver (for checkAuthInfo)!";
        kDebug(7019) << reply.error().name() << reply.error().message();
        return -1;
    }

    if (!loop.waitForResult(reply.value())) {
        kWarning(7019) << "kded_kpasswdserver died while waiting for reply!";
        return -1;
    }

    if (loop.authInfo().isModified()) {
        info = loop.authInfo();
    }

    kDebug(7019) << "username=" << info.username << "password=[hidden]";

    return loop.seqNr();
}

qlonglong KPasswdServer::legacyCheckAuthInfo(KIO::AuthInfo &info, qlonglong windowId,
                                             qlonglong usertime)
{
    QByteArray params;
    QDataStream stream(&params, QIODevice::WriteOnly);
    stream << info;
    QDBusPendingReply<QByteArray, qlonglong> reply = m_interface->checkAuthInfo(params, windowId,
                                                                                usertime);
    reply.waitForFinished();
    if (reply.isValid()) {
        AuthInfo authResult;
        QDataStream stream2(reply.argumentAt<0>());
        stream2 >> authResult;
        if (authResult.isModified()) {
            info = authResult;
        }
        return reply.argumentAt<1>();
    }
    return -1;
}

qlonglong KPasswdServer::queryAuthInfo(KIO::AuthInfo &info, const QString &errorMsg,
                                       qlonglong windowId, qlonglong seqNr,
                                       qlonglong usertime)
{
    kDebug(7019) << "window-id=" << windowId;

    // create the loop for waiting for a result before sending the request
    KPasswdServerLoop loop;
    connect(m_interface,
            SIGNAL(queryAuthInfoAsyncResult(qlonglong, qlonglong, const KIO::AuthInfo &)),
            &loop, SLOT(slotQueryResult(qlonglong, qlonglong, const KIO::AuthInfo &)));

    QDBusReply<qlonglong> reply = m_interface->queryAuthInfoAsync(info, errorMsg,
                                                                  windowId, seqNr,
                                                                  usertime);
    if (!reply.isValid()) {
        // backwards compatibility for old kpasswdserver
        if (reply.error().type() == QDBusError::UnknownMethod) {
            qlonglong res = legacyQueryAuthInfo(info, errorMsg, windowId, seqNr,
                                                usertime);
            if (res > 0) {
                return res;
            }
        }

        kWarning(7019) << "Can't communicate with kded_kpasswdserver (for queryAuthInfo)!";
        kDebug(7019) << reply.error().name() << reply.error().message();
        return -1;
    }

    if (!loop.waitForResult(reply.value())) {
        kWarning(7019) << "kded_kpasswdserver died while waiting for reply!";
        return -1;
    }

    if (loop.authInfo().isModified()) {
        info = loop.authInfo();
    }

    kDebug(7019) << "username=" << info.username << "password=[hidden]";

    return loop.seqNr();
}

qlonglong KPasswdServer::legacyQueryAuthInfo(KIO::AuthInfo &info, const QString &errorMsg,
                                             qlonglong windowId, qlonglong seqNr,
                                             qlonglong usertime)
{
    QByteArray params;
    QDataStream stream(&params, QIODevice::WriteOnly);
    stream << info;
    QDBusPendingReply<QByteArray, qlonglong> reply = m_interface->queryAuthInfo(params, errorMsg,
                                                                                windowId, seqNr,
                                                                                usertime);
    if (reply.isValid()) {
        AuthInfo authResult;
        QDataStream stream2(reply.argumentAt<0>());
        stream2 >> authResult;
        if (authResult.isModified()) {
            info = authResult;
        }
        return reply.argumentAt<1>();
    }
    return -1;
}

void KPasswdServer::addAuthInfo(const KIO::AuthInfo &info, qlonglong windowId)
{
    QByteArray params;
    QDataStream stream(&params, QIODevice::WriteOnly);
    stream << info;
    m_interface->addAuthInfo(params, windowId);
}

void KPasswdServer::removeAuthInfo(const QString &host, const QString &protocol,
                                   const QString &user)
{
    m_interface->removeAuthInfo(host, protocol, user);
}

}

#include "kpasswdserver_p.moc"
