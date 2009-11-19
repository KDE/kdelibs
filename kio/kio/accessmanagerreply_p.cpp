/*
 * This file is part of the KDE project.
 *
 * Copyright (C) 2008 Alex Merry <alex.merry @ kdemail.net>
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

#include "accessmanagerreply_p.h"
#include "accessmanager.h"
#include "job.h"

#include <kdebug.h>
#include <klocale.h>

#include <QSslConfiguration>
#include <QTimer>
#include <QPointer>

namespace KDEPrivate {

class AccessManagerReply::AccessManagerReplyPrivate
{
public:
    AccessManagerReplyPrivate(AccessManagerReply *qq)
    : q(qq),
      m_metaDataRead(false) {}

    void _k_redirection(KIO::Job *job, const KUrl &url);
    void _k_percent(KJob *job, unsigned long percent);

    AccessManagerReply *q;

    QPointer<KIO::Job> m_kioJob;
    QByteArray m_data;
    bool m_metaDataRead;
};

AccessManagerReply::AccessManagerReply(const QNetworkAccessManager::Operation &op, const QNetworkRequest &request, KIO::Job *kioJob, QObject *parent)
                   :QNetworkReply(parent), d(new AccessManagerReply::AccessManagerReplyPrivate(this))

{
    d->m_kioJob = kioJob;
    setRequest(request);
    setOpenMode(QIODevice::ReadOnly);
    setUrl(request.url());
    setOperation(op);

    if (!request.sslConfiguration().isNull()) {
        setSslConfiguration(request.sslConfiguration());
        kDebug() << "QSslConfiguration not supported (currently).";
    }

    if (!kioJob) { // a blocked request
        setError(QNetworkReply::OperationCanceledError, i18n("Blocked request."));
        QTimer::singleShot(0, this, SIGNAL(finished()));
    } else {
        connect(kioJob, SIGNAL(redirection(KIO::Job*, const KUrl&)), SLOT(_k_redirection(KIO::Job*, const KUrl&)));
        connect(kioJob, SIGNAL(percent(KJob*, unsigned long)), SLOT(_k_percent(KJob*, unsigned long)));
        connect(kioJob, SIGNAL(data(KIO::Job *, const QByteArray &)),
            SLOT(appendData(KIO::Job *, const QByteArray &)));
        connect(kioJob, SIGNAL(result(KJob *)), SLOT(jobDone(KJob *)));
        connect(kioJob, SIGNAL(mimetype(KIO::Job *, const QString&)),
            SLOT(setMimeType(KIO::Job *, const QString&)));
    }
}

AccessManagerReply::~AccessManagerReply()
{
    delete d;
}

void AccessManagerReply::abort()
{
    if (d->m_kioJob) {
        d->m_kioJob->kill();
        d->m_kioJob->deleteLater();
    }
}

qint64 AccessManagerReply::bytesAvailable() const
{
    return QNetworkReply::bytesAvailable() + d->m_data.length();
}

qint64 AccessManagerReply::readData(char *data, qint64 maxSize)
{
    const qint64 length = qMin(qint64(d->m_data.length()), maxSize);
    if (length) {
        qMemCopy(data, d->m_data.constData(), length);
        d->m_data.remove(0, length);
    }

    return length;
}

void AccessManagerReply::appendData(KIO::Job *kioJob, const QByteArray &data)
{
    if (!d->m_metaDataRead) {
        const QString responseCode = kioJob->queryMetaData("responsecode");
        if (!responseCode.isEmpty()) 
            setAttribute(QNetworkRequest::HttpStatusCodeAttribute, responseCode.toInt());

        const QString headers = kioJob->queryMetaData("HTTP-Headers");
        if (!headers.isEmpty()) {
            QStringListIterator it (headers.split('\n'));
            while (it.hasNext()) {
                const QStringList headerPair = it.next().split(QLatin1String(":"));
                if (headerPair.size() == 2) {
                    setRawHeader(headerPair.at(0).trimmed().toUtf8(), headerPair.at(1).trimmed().toUtf8());
                }
            }
        }

        setAttribute(static_cast<QNetworkRequest::Attribute>(KIO::AccessManager::MetaData),
                     kioJob->metaData().toVariant());
        d->m_metaDataRead = true;
    }

    d->m_data += data;
    emit readyRead();
}

void AccessManagerReply::setMimeType(KIO::Job *kioJob, const QString &mimeType)
{
    Q_UNUSED(kioJob);
    setHeader(QNetworkRequest::ContentTypeHeader, mimeType.toUtf8());
}

void AccessManagerReply::jobDone(KJob *kJob)
{
    const int errcode = kJob->error();
    switch (errcode)
    {
        case 0:
            setError(QNetworkReply::NoError, kJob->errorText());
            kDebug() << "0 -> QNetworkReply::NoError";
            break;
        case KIO::ERR_COULD_NOT_CONNECT:
            setError(QNetworkReply::ConnectionRefusedError, kJob->errorText());
            kDebug() << "KIO::ERR_COULD_NOT_CONNECT -> QNetworkReply::ConnectionRefusedError";
            break;
        case KIO::ERR_UNKNOWN_HOST:
            setError(QNetworkReply::HostNotFoundError, kJob->errorText());
            kDebug() << "KIO::ERR_UNKNOWN_HOST -> QNetworkReply::HostNotFoundError";
            break;
        case KIO::ERR_SERVER_TIMEOUT:
            setError(QNetworkReply::TimeoutError, kJob->errorText());
            kDebug() << "KIO::ERR_SERVER_TIMEOUT -> QNetworkReply::TimeoutError";
            break;
        case KIO::ERR_USER_CANCELED:
        case KIO::ERR_ABORTED:
            setError(QNetworkReply::OperationCanceledError, kJob->errorText());
            kDebug() << "KIO::ERR_ABORTED -> QNetworkReply::OperationCanceledError";
            break;
        case KIO::ERR_UNKNOWN_PROXY_HOST:
            setError(QNetworkReply::ProxyNotFoundError, kJob->errorText());
            kDebug() << "KIO::UNKNOWN_PROXY_HOST -> QNetworkReply::ProxyNotFoundError";
            break;
        case KIO::ERR_ACCESS_DENIED:
            setError(QNetworkReply::ContentAccessDenied, kJob->errorText());
            kDebug() << "KIO::ERR_ACCESS_DENIED -> QNetworkReply::ContentAccessDenied";
            break;
        case KIO::ERR_WRITE_ACCESS_DENIED:
            setError(QNetworkReply::ContentOperationNotPermittedError, kJob->errorText());
            kDebug() << "KIO::ERR_WRITE_ACCESS_DENIED -> QNetworkReply::ContentOperationNotPermittedError";
            break;
        case KIO::ERR_NO_CONTENT:
        case KIO::ERR_DOES_NOT_EXIST:
            setError(QNetworkReply::ContentNotFoundError, kJob->errorText());
            kDebug() << "KIO::ERR_DOES_NOT_EXIST -> QNetworkReply::ContentNotFoundError";
            break;
        case KIO::ERR_COULD_NOT_AUTHENTICATE:
            setError(QNetworkReply::AuthenticationRequiredError, kJob->errorText());
            kDebug() << errcode;
            break;
        case KIO::ERR_UNSUPPORTED_PROTOCOL:
        case KIO::ERR_NO_SOURCE_PROTOCOL:
            setError(QNetworkReply::ProtocolUnknownError, kJob->errorText());
            kDebug() << errcode;
            break;
        case KIO::ERR_UNSUPPORTED_ACTION:
            setError(QNetworkReply::ProtocolInvalidOperationError, kJob->errorText());
            kDebug() << kJob->error();
            break;
        default:
            setError(QNetworkReply::UnknownNetworkError, errorString());
            kDebug() << errcode;
    }

    setAttribute(static_cast<QNetworkRequest::Attribute>(KIO::AccessManager::KioError), errcode);
    emit finished();
}

void AccessManagerReply::AccessManagerReplyPrivate::_k_redirection(KIO::Job* job, const KUrl& url)
{
    job->kill();
    m_kioJob = 0;
    
    // unfortunately we don't get HTTP response code for redirection, so for
    // temporary one assume code 302 which is most often used
    if (q->attribute(QNetworkRequest::HttpStatusCodeAttribute).isNull())
        q->setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 302);

    q->setAttribute(QNetworkRequest::RedirectionTargetAttribute, QUrl(url));
    emit q->finished();
}

void AccessManagerReply::AccessManagerReplyPrivate::_k_percent(KJob *job, unsigned long percent)
{
    qulonglong bytes = job->totalAmount(KJob::Bytes);
    emit q->downloadProgress(bytes * ((double)percent / 100), bytes);
}

}

#include "accessmanagerreply_p.moc"
