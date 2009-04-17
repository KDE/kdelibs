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
 *
 **/

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>

#include "kpasswdserverloop_p.h"

namespace KIO
{

KPasswdServerLoop::KPasswdServerLoop() : m_seqNr(-1)
{
    connect(QDBusConnection::sessionBus().interface(),
            SIGNAL(serviceOwnerChanged(QString, QString, QString)),
            this,
            SLOT(slotServiceOwnerChanged(QString, QString, QString)));
}

KPasswdServerLoop::~KPasswdServerLoop()
{
}

bool KPasswdServerLoop::waitForResult(qlonglong requestId)
{
    m_requestId = requestId;
    m_seqNr = -1;
    m_authInfo = AuthInfo();
    return (exec() == 0);
}

qlonglong KPasswdServerLoop::seqNr() const
{
    return m_seqNr;
}

const AuthInfo &KPasswdServerLoop::authInfo() const
{
    return m_authInfo;
}

void KPasswdServerLoop::slotQueryResult(qlonglong requestId, qlonglong seqNr,
                                       const KIO::AuthInfo &authInfo)
{
    if (m_requestId == requestId) {
        m_seqNr = seqNr;
        m_authInfo = authInfo;
        exit(0);
    }
}

void KPasswdServerLoop::slotServiceOwnerChanged(const QString &name, const QString &oldOwner,
                                               const QString &newOwner)
{
    Q_UNUSED(oldOwner);

    if (newOwner.isEmpty() && name == "org.kde.kded") {
        exit(-1);
    }
}

}

#include "kpasswdserverloop_p.moc"
