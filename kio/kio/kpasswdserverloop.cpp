/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2009 Michael Leupold <lemma@confuego.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) version 3, or any
 *  later version accepted by the membership of KDE e.V. (or its
 *  successor approved by the membership of KDE e.V.), which shall
 *  act as a proxy defined in Section 6 of version 3 of the license.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

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
