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

#ifndef PASSWDSERVERLOOP_P_H
#define PASSWDSERVERLOOP_P_H

#include <kio/authinfo.h>
#include <QtCore/QByteArray>
#include <QtCore/QEventLoop>

namespace KIO {

// Wait for the result of an asynchronous D-Bus request to KPasswdServer.
// Objects of this class are one-way ie. as soon as they have received
// a result you can't call waitForResult() again.
class PasswdServerLoop : public QEventLoop
{
    Q_OBJECT

public:
    PasswdServerLoop();
    virtual ~PasswdServerLoop();
    bool waitForResult(qlonglong requestId);

    qlonglong seqNr() const;
    const AuthInfo &authInfo() const;

public Q_SLOTS:
    void slotQueryResult(qlonglong requestId, qlonglong seqNr, const KIO::AuthInfo &authInfo);
    void slotServiceOwnerChanged(const QString &name, const QString &oldOwner,
                                 const QString &newOwner);

private:
    qlonglong m_requestId;
    qlonglong m_seqNr;
    AuthInfo m_authInfo;
};

}

#endif
