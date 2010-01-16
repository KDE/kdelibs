/*
 * This file is part of the KDE project.
 *
 * Copyright (C) 2008 - 2009 Urs Wolfer <uwolfer @ kde.org>
 * Copyright (C) 2007 Trolltech ASA
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

#include "accessmanager.h"

#include "accessmanagerreply_p.h"

#include <kdebug.h>
#include <kio/job.h>
#include <kio/scheduler.h>

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>


namespace KIO {

class AccessManager::AccessManagerPrivate
{
public:
    AccessManagerPrivate():externalContentAllowed(true) {}
    bool externalContentAllowed;
    static KIO::MetaData metaDataForRequest(QNetworkRequest request);
};

}

using namespace KIO;

AccessManager::AccessManager(QObject *parent)
    : QNetworkAccessManager(parent), d(new AccessManager::AccessManagerPrivate())
{
}

AccessManager::~AccessManager()
{
    delete d;
}

void AccessManager::setExternalContentAllowed(bool allowed)
{
    d->externalContentAllowed = allowed;
}

bool AccessManager::isExternalContentAllowed() const
{
    return d->externalContentAllowed;
}

QNetworkReply *AccessManager::createRequest(Operation op, const QNetworkRequest &req, QIODevice *outgoingData)
{
    KIO::SimpleJob *kioJob = 0;

    if ( !d->externalContentAllowed && req.url().scheme() != "file" && !req.url().scheme().isEmpty() ) {
        kDebug() << "Blocked: " << req.url().scheme() <<  req.url();
        /* if kioJob equals zero, the AccessManagerReply will block the request */
        return new KDEPrivate::AccessManagerReply(op, req, kioJob, this);
    }

    switch (op) {
        case HeadOperation: {
            kDebug() << "HeadOperation:" << req.url();
            kioJob = KIO::mimetype(req.url(), KIO::HideProgressInfo);
            break;
        }
        case GetOperation: {
            kDebug() << "GetOperation:" << req.url();
            kioJob = KIO::get(req.url(), KIO::NoReload, KIO::HideProgressInfo);
            break;
        }
        case PutOperation: {
            kDebug() << "PutOperation:" << req.url();
            kioJob = KIO::put(req.url(), -1, KIO::HideProgressInfo);
            break;
        }
        case PostOperation: {
            kDebug() << "PostOperation:" << req.url();
            kioJob = KIO::http_post(req.url(), outgoingData->readAll(), KIO::HideProgressInfo);
            break;
        }
        default:
            kDebug() << "Unknown operation";
            return 0;
    }

    KIO::Scheduler::scheduleJob(kioJob);
    KDEPrivate::AccessManagerReply *reply = new KDEPrivate::AccessManagerReply(op, req, kioJob, this);

    kioJob->addMetaData(d->metaDataForRequest(req));

    if ( op == PostOperation && !kioJob->metaData().contains("content-type"))  {
        QString contentType (QLatin1String("Content-Type: "));
        QVariant header = req.header(QNetworkRequest::ContentTypeHeader);

        if (header.isValid())
            contentType += header.toString();
        else
            contentType += QLatin1String("application/x-www-form-urlencoded");

        kioJob->addMetaData("content-type", contentType);
    }

    //kDebug () << "Job '" << kioJob << "' started...";
    return reply;
}


KIO::MetaData AccessManager::AccessManagerPrivate::metaDataForRequest(QNetworkRequest request)
{
    KIO::MetaData metaData;

    // Add the user-specified meta data first...
    QVariant userMetaData = request.attribute (static_cast<QNetworkRequest::Attribute>(MetaData));
    if (userMetaData.isValid() && userMetaData.type() == QVariant::Map) {
      metaData += userMetaData.toMap();
    }

    metaData.insert("PropagateHttpHeader", "true");

    metaData.insert("UserAgent", request.rawHeader("User-Agent"));
    request.setRawHeader("User-Agent", QByteArray());

    metaData.insert("accept", request.rawHeader("Accept"));
    request.setRawHeader("Accept", QByteArray());

    request.setRawHeader("content-length", QByteArray());
    request.setRawHeader("Connection", QByteArray());

    QString additionHeaders;
    QListIterator<QByteArray> headersIt (request.rawHeaderList());

    while (headersIt.hasNext()) {
        const QByteArray key = headersIt.next();
        const QByteArray value = request.rawHeader(key);

        if (value.isNull())
            continue;

        // createRequest() checks later for existence "content-type" metadata
        if (QString::compare(key, QLatin1String("Content-Type"), Qt::CaseInsensitive) == 0) {
            metaData.insert("content-type", value);
            continue;
        }

        if (additionHeaders.length() > 0) {
            additionHeaders += QLatin1String("\r\n");
        }

        additionHeaders += key;
        additionHeaders += QLatin1String(": ");
        additionHeaders += value;
    }

    metaData.insert("customHTTPHeader", additionHeaders);
    return metaData;
}

#include "accessmanager.moc"
