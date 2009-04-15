/*
 * This file is part of the KDE project.
 *
 * Copyright (C) 2008 - 2009 Urs Wolfer <uwolfer @ kde.org>
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
 *
 */

#ifndef KIO_ACCESSMANAGER_H
#define KIO_ACCESSMANAGER_H

#include <QtNetwork/QNetworkAccessManager>

#include <kio/global.h>

/**
 * KDE implementation of QNetworkAccessManager. Use this class instead of QNetworkAccessManager
 * for any KDE application since it uses KIO for network operations.
 * Please note that this class is not though as replacement for KIO API! Just use if in places where
 * it is required, such as network binding with Qt classes (one example is QtWebKit); i.e. API
 * requires a QNetworkAccessManager.
 * @since 4.3
 */

namespace KIO {

class KIO_EXPORT AccessManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    AccessManager(QObject *parent);
    virtual ~AccessManager();

protected:
    virtual QNetworkReply *createRequest(Operation op, const QNetworkRequest &req, QIODevice *outgoingData = 0);

private:
    class AccessManagerPrivate;
    AccessManagerPrivate* const d;
};

}

#endif // KIO_ACCESSMANAGER_H
