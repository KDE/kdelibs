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

#include <QtNetwork/QSslConfiguration>
#include <QtCore/QTimer>
#include <QtCore/QPointer>

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

    QPointer<KIO::SimpleJob> m_kioJob;
    QByteArray m_data;
    bool m_metaDataRead;
};

AccessManagerReply::AccessManagerReply(const QNetworkAccessManager::Operation &op, const QNetworkRequest &request, KIO::SimpleJob *kioJob, QObject *parent)
                   :QNetworkReply(parent), d(new AccessManagerReply::AccessManagerReplyPrivate(this))

{
    d->m_kioJob = kioJob;
    setRequest(request);
    setOpenMode(QIODevice::ReadOnly);
    setUrl(request.url());
    setOperation(op);

    if (!request.sslConfiguration().isNull()) {
        setSslConfiguration(request.sslConfiguration());
    }

    if (!kioJob) { // a blocked request
        setError(QNetworkReply::OperationCanceledError, i18n("Blocked request."));
        QTimer::singleShot(0, this, SIGNAL(finished()));
    } else {
        connect(kioJob, SIGNAL(redirection(KIO::Job*, const KUrl&)), SLOT(_k_redirection(KIO::Job*, const KUrl&)));
        connect(kioJob, SIGNAL(percent(KJob*, unsigned long)), SLOT(_k_percent(KJob*, unsigned long)));
        connect(kioJob, SIGNAL(result(KJob *)), SLOT(jobDone(KJob *)));

        if (!qobject_cast<KIO::StatJob*>(kioJob)) {
            connect(kioJob, SIGNAL(data(KIO::Job *, const QByteArray &)),
                SLOT(appendData(KIO::Job *, const QByteArray &)));
            connect(kioJob, SIGNAL(mimetype(KIO::Job *, const QString&)),
                SLOT(setMimeType(KIO::Job *, const QString&)));
        }
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
    }

    d->m_data.clear();
    d->m_metaDataRead = false;
}

qint64 AccessManagerReply::bytesAvailable() const
{
    return (QNetworkReply::bytesAvailable() + d->m_data.length());
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

void AccessManagerReply::readHttpResponseHeaders(KIO::Job *job)
{
    if (!d->m_metaDataRead) {
        // Set the HTTP status code...
        const QString responseCode = job->queryMetaData("responsecode");
        if (!responseCode.isEmpty())
            setAttribute(QNetworkRequest::HttpStatusCodeAttribute, responseCode.toInt());

        // Set the encryption attribute and values...
        QSslConfiguration sslConfig;
        const bool isEncrypted = KIO::Integration::sslConfigFromMetaData(job->metaData(), sslConfig);
        setAttribute(QNetworkRequest::ConnectionEncryptedAttribute, isEncrypted);
        if (isEncrypted)
            setSslConfiguration(sslConfig);

        // Set the raw header information...
        const QString headers = job->queryMetaData("HTTP-Headers");
        if (!headers.isEmpty()) {
            QStringListIterator it (headers.split('\n'));
            while (it.hasNext()) {
                QStringList headerPair = it.next().split(QLatin1String(":"));
                if (headerPair.size() < 2)
                    continue;
                // Skip setting cookies since they are automatically handled by kio_http...
                if (headerPair.first().startsWith("set-cookie", Qt::CaseInsensitive))
                    continue;
                if (headerPair.first().startsWith("content-type", Qt::CaseInsensitive)) {
                    const QString mimeType = header(QNetworkRequest::ContentTypeHeader).toString();
                    if (!headerPair.at(1).contains(mimeType, Qt::CaseInsensitive)) {
                        const int index = headerPair.at(1).indexOf(QLatin1Char(';'));
                        if (index == -1)
                            headerPair[1] = mimeType;
                        else
                            headerPair[1].replace(0, index, mimeType);
                    }
                }
                //kDebug(7044) << "Adding header:" << headerPair.at(0) << ":" << headerPair.at(1);
                setRawHeader(headerPair.at(0).trimmed().toUtf8(), headerPair.at(1).trimmed().toUtf8());
            }
        }

        // Set the returned meta data as attribute...
        setAttribute(static_cast<QNetworkRequest::Attribute>(KIO::AccessManager::MetaData),
                     job->metaData().toVariant());
        d->m_metaDataRead = true;
    }
}

void AccessManagerReply::appendData(KIO::Job *kioJob, const QByteArray &data)
{
    readHttpResponseHeaders(kioJob);
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
    int errcode = kJob->error();
    switch (errcode)
    {
        case 0:
            setError(QNetworkReply::NoError, kJob->errorText());
            //kDebug( 7044 ) << "0 -> QNetworkReply::NoError";
            break;
        case KIO::ERR_COULD_NOT_CONNECT:
            setError(QNetworkReply::ConnectionRefusedError, kJob->errorText());
            kDebug( 7044 ) << "KIO::ERR_COULD_NOT_CONNECT -> KIO::ERR_COULD_NOT_CONNECT";
            break;
        case KIO::ERR_UNKNOWN_HOST:
            setError(QNetworkReply::HostNotFoundError, kJob->errorText());
            kDebug( 7044 ) << "KIO::ERR_UNKNOWN_HOST -> QNetworkReply::HostNotFoundError";
            break;
        case KIO::ERR_SERVER_TIMEOUT:
            setError(QNetworkReply::TimeoutError, kJob->errorText());
            kDebug( 7044 ) << "KIO::ERR_SERVER_TIMEOUT -> QNetworkReply::TimeoutError";
            break;
        case KIO::ERR_USER_CANCELED:
        case KIO::ERR_ABORTED:
            setError(QNetworkReply::OperationCanceledError, kJob->errorText());
            kDebug( 7044 ) << "KIO::ERR_ABORTED -> QNetworkReply::OperationCanceledError";
            break;
        case KIO::ERR_UNKNOWN_PROXY_HOST:
            setError(QNetworkReply::ProxyNotFoundError, kJob->errorText());
            kDebug( 7044 ) << "KIO::UNKNOWN_PROXY_HOST -> QNetworkReply::ProxyNotFoundError";
            break;
        case KIO::ERR_ACCESS_DENIED:
            setError(QNetworkReply::ContentAccessDenied, kJob->errorText());
            kDebug( 7044 ) << "KIO::ERR_ACCESS_DENIED -> QNetworkReply::ContentAccessDenied";
            break;
        case KIO::ERR_WRITE_ACCESS_DENIED:
            setError(QNetworkReply::ContentOperationNotPermittedError, kJob->errorText());
            kDebug( 7044 ) << "KIO::ERR_WRITE_ACCESS_DENIED -> QNetworkReply::ContentOperationNotPermittedError";
            break;
        case KIO::ERR_NO_CONTENT:
        case KIO::ERR_DOES_NOT_EXIST:
            setError(QNetworkReply::ContentNotFoundError, kJob->errorText());
            kDebug( 7044 ) << "KIO::ERR_DOES_NOT_EXIST -> QNetworkReply::ContentNotFoundError";
            break;
        case KIO::ERR_COULD_NOT_AUTHENTICATE:
            setError(QNetworkReply::AuthenticationRequiredError, kJob->errorText());
            kDebug( 7044 ) << errcode;
            break;
        case KIO::ERR_UNSUPPORTED_PROTOCOL:
        case KIO::ERR_NO_SOURCE_PROTOCOL:
            setError(QNetworkReply::ProtocolUnknownError, kJob->errorText());
            kDebug( 7044 ) << errcode;
            break;
        case KIO::ERR_UNSUPPORTED_ACTION:
            setError(QNetworkReply::ProtocolInvalidOperationError, kJob->errorText());
            kDebug( 7044 ) << errcode;
            break;
        default:
            setError(QNetworkReply::UnknownNetworkError, kJob->errorText());
            kDebug( 7044 ) << errcode;
    }

    QUrl redirectUrl = attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (redirectUrl.isValid()) {
        readHttpResponseHeaders(d->m_kioJob);
        //kDebug( 7044 ) << "HTTP Status code:" << attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        //kDebug( 7044 ) << "Redirecting to" << redirectUrl;
    }

    setAttribute(static_cast<QNetworkRequest::Attribute>(KIO::AccessManager::KioError), errcode);
    if (errcode)
        emit error(error());
    emit finished();
}

void AccessManagerReply::AccessManagerReplyPrivate::_k_redirection(KIO::Job* job, const KUrl& url)
{   
    Q_UNUSED(job);
    q->setAttribute(QNetworkRequest::RedirectionTargetAttribute, QUrl(url));
}

void AccessManagerReply::AccessManagerReplyPrivate::_k_percent(KJob *job, unsigned long percent)
{
    qulonglong bytes = job->totalAmount(KJob::Bytes);
    emit q->downloadProgress(bytes * ((double)percent / 100), bytes);
}
}

#include "accessmanagerreply_p.moc"
