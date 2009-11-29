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
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <QtCore/QUrl>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusReply>


namespace KIO {

class AccessManager::AccessManagerPrivate
{
public:
    AccessManagerPrivate():externalContentAllowed(true) {}
    KIO::MetaData metaDataForRequest(QNetworkRequest request);

    bool externalContentAllowed;
    KIO::MetaData requestMetaData;
    KIO::MetaData sessionMetaData;
};

namespace Integration {

class CookieJar::CookieJarPrivate
{
public:
  CookieJarPrivate(): windowId((WId)-1), enabled(true) {}

  WId windowId;
  bool enabled;
};

}

}

using namespace KIO;

AccessManager::AccessManager(QObject *parent)
              :QNetworkAccessManager(parent), d(new AccessManager::AccessManagerPrivate())
{
    // KDE Cookiejar (KCookieJar) integration...
    setCookieJar(new KIO::Integration::CookieJar);
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

void AccessManager::setCookieJarWindowId(WId id)
{
    KIO::Integration::CookieJar *jar = qobject_cast<KIO::Integration::CookieJar *> (cookieJar());
    if (jar) {
        jar->setWindowId(id);
        d->sessionMetaData.insert(QLatin1String("window-id"), QString::number((qlonglong)id));
    }
}

WId AccessManager::cookieJarWindowid() const
{
    KIO::Integration::CookieJar *jar = qobject_cast<KIO::Integration::CookieJar *> (cookieJar());
    if (jar)
        return jar->windowId();

    return 0;
}

KIO::MetaData& AccessManager::requestMetaData()
{
    return d->requestMetaData;
}

KIO::MetaData& AccessManager::sessionMetaData()
{
    return d->sessionMetaData;
}

QNetworkReply *AccessManager::createRequest(Operation op, const QNetworkRequest &req, QIODevice *outgoingData)
{
    KIO::SimpleJob *kioJob = 0;

    if ( !d->externalContentAllowed && req.url().scheme() != "file" && !req.url().scheme().isEmpty() ) {
        kDebug( 7044 ) << "Blocked: " << req.url().scheme() <<  req.url();
        /* if kioJob equals zero, the AccessManagerReply will block the request */
        return new KDEPrivate::AccessManagerReply(op, req, kioJob, this);
    }

    switch (op) {
        case HeadOperation: {
            kDebug( 7044 ) << "HeadOperation:" << req.url();
            kioJob = KIO::mimetype(req.url(), KIO::HideProgressInfo);
            break;
        }
        case GetOperation: {
            kDebug( 7044 ) << "GetOperation:" << req.url();
            kioJob = KIO::get(req.url(), KIO::NoReload, KIO::HideProgressInfo);
            break;
        }
        case PutOperation: {
            kDebug( 7044 ) << "PutOperation:" << req.url();
            kioJob = KIO::put(req.url(), -1, KIO::HideProgressInfo);
            break;
        }
        case PostOperation: {
            kDebug( 7044 ) << "PostOperation:" << req.url();
            kioJob = KIO::http_post(req.url(), outgoingData->readAll(), KIO::HideProgressInfo);
            break;
        }
        default:
            kDebug( 7044 ) << "Unknown operation";
            return 0;
    }

    KIO::Scheduler::scheduleJob(kioJob);
    KDEPrivate::AccessManagerReply *reply = new KDEPrivate::AccessManagerReply(op, req, kioJob, this);

    kioJob->addMetaData(d->metaDataForRequest(req));

    if ( op == PostOperation && !kioJob->metaData().contains("content-type"))  {
        QVariant header = req.header(QNetworkRequest::ContentTypeHeader);
        if (header.isValid())
          kioJob->addMetaData("content-type",
                              QString::fromLatin1("Content-Type: %1").arg(header.toString()));
        else
          kioJob->addMetaData("content-type", "Content-Type: application/x-www-form-urlencoded");
    }

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
    Q_FOREACH(const QByteArray &headerKey, request.rawHeaderList()) {
        const QByteArray value = request.rawHeader(headerKey);
        if (value.isNull())
            continue;

        // createRequest() checks later for existence "content-type" metadata
        if (headerKey=="Content-Type") {
            metaData.insert("content-type", value);
            continue;
        }

        if (additionHeaders.length() > 0) {
            additionHeaders += "\r\n";
        }
        additionHeaders += headerKey + ": " + value;
    }
    metaData.insert("customHTTPHeader", additionHeaders);

    // Append per request meta data, if any...
    if (!requestMetaData.isEmpty())
        metaData += requestMetaData;

    // Append per session meta data, if any...
    if (!sessionMetaData.isEmpty())
        metaData += sessionMetaData;

    // Clear per request meta data...
    requestMetaData.clear();

    return metaData;
}


using namespace KIO::Integration;

CookieJar::CookieJar(QObject* parent)
          :QNetworkCookieJar(parent), d(new CookieJar::CookieJarPrivate) {
    reparseConfiguration();
}

CookieJar::~CookieJar() {
    delete d;
}

WId CookieJar::windowId() const {
    return d->windowId;
}

QList<QNetworkCookie> CookieJar::cookiesForUrl(const QUrl &url) const {
    QList<QNetworkCookie> cookieList;

    if (d->enabled) {
        QDBusInterface kcookiejar("org.kde.kded", "/modules/kcookiejar", "org.kde.KCookieServer");
        QDBusReply<QString> reply = kcookiejar.call("findDOMCookies", url.toString(), (qlonglong)d->windowId);

        if (reply.isValid()) {
            cookieList << reply.value().toUtf8();
            //kDebug() << url.host() << reply.value();
        } else {
            kWarning() << "Unable to communicate with the cookiejar!";
        }
    }

    return cookieList;
}

bool CookieJar::setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url) {
    if (d->enabled) {
        QDBusInterface kcookiejar("org.kde.kded", "/modules/kcookiejar", "org.kde.KCookieServer");

        QByteArray cookieHeader;
        Q_FOREACH(const QNetworkCookie &cookie, cookieList) {
            cookieHeader = "Set-Cookie: ";
            cookieHeader += cookie.toRawForm();
            kcookiejar.call("addCookies", url.toString(), cookieHeader, (qlonglong)d->windowId);
            //kDebug() << "[" << d->windowId << "] Got Cookie: " << cookieHeader << " from " << url;
        }

        return !kcookiejar.lastError().isValid();
    }

    return false;
}

void CookieJar::setWindowId(WId id) {
    d->windowId = id;
}

void CookieJar::reparseConfiguration() {
    KConfigGroup cfg = KSharedConfig::openConfig("kcookiejarrc", KConfig::NoGlobals)->group("Cookie Policy");
    d->enabled = cfg.readEntry("Cookies", true);
}


#include "accessmanager.moc"
