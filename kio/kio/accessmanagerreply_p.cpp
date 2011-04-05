/*
 * This file is part of the KDE project.
 *
 * Copyright (C) 2008 Alex Merry <alex.merry @ kdemail.net>
 * Copyright (C) 2008 - 2009 Urs Wolfer <uwolfer @ kde.org>
 * Copyright (C) 2009 - 2010 Dawit Alemayehu <adawit @ kde.org>
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
#include <kprotocolinfo.h>

#include <QtNetwork/QSslConfiguration>

#define QL1S(x)  QLatin1String(x)
#define QL1C(x)  QLatin1Char(x)


static bool accessManager_isLocalRequest(const KUrl& url)
{
    const QString scheme (url.protocol());
    return (KProtocolInfo::isKnownProtocol(scheme) &&
            KProtocolInfo::protocolClass(scheme).compare(QL1S(":local"), Qt::CaseInsensitive) == 0);
}

namespace KDEPrivate {

AccessManagerReply::AccessManagerReply(const QNetworkAccessManager::Operation &op,
                                       const QNetworkRequest &request,
                                       KIO::SimpleJob *kioJob,
                                       QObject *parent)
                   :QNetworkReply(parent),
                    m_metaDataRead(false),
                    m_ignoreContentDisposition(false)

{
    m_kioJob = kioJob;
    setRequest(request);
    setOpenMode(QIODevice::ReadOnly);
    setUrl(request.url());
    setOperation(op);
    setError(NoError, QString());

    if (!request.sslConfiguration().isNull())
        setSslConfiguration(request.sslConfiguration());

    if (kioJob) {
        connect(kioJob, SIGNAL(redirection(KIO::Job*, const KUrl&)), SLOT(slotRedirection(KIO::Job*, const KUrl&)));
        connect(kioJob, SIGNAL(percent(KJob*, unsigned long)), SLOT(slotPercent(KJob*, unsigned long)));

        if (qobject_cast<KIO::StatJob*>(kioJob)) {
            connect(kioJob, SIGNAL(result(KJob *)), SLOT(slotStatResult(KJob *)));
        } else {
            connect(kioJob, SIGNAL(result(KJob *)), SLOT(slotResult(KJob *)));
            connect(kioJob, SIGNAL(data(KIO::Job *, const QByteArray &)),
                SLOT(slotData(KIO::Job *, const QByteArray &)));
            connect(kioJob, SIGNAL(mimetype(KIO::Job *, const QString&)),
                SLOT(slotMimeType(KIO::Job *, const QString&)));
        }
    }
}

AccessManagerReply::~AccessManagerReply()
{
}

void AccessManagerReply::abort()
{
    if (m_kioJob)
        m_kioJob->kill();

    m_data.clear();
    m_metaDataRead = false;
}

qint64 AccessManagerReply::bytesAvailable() const
{
    return (QNetworkReply::bytesAvailable() + m_data.length());
}

qint64 AccessManagerReply::readData(char *data, qint64 maxSize)
{
    const qint64 length = qMin(qint64(m_data.length()), maxSize);

    if (length) {
        qMemCopy(data, m_data.constData(), length);
        m_data.remove(0, length);
    }

    return length;
}

void AccessManagerReply::setIgnoreContentDisposition(bool on)
{
    kDebug(7044) << on;
    m_ignoreContentDisposition = on;
}

void AccessManagerReply::setStatus(const QString& message, QNetworkReply::NetworkError code)
{
    setError(code, message);
}

void AccessManagerReply::putOnHold()
{
    if (!m_kioJob)
        return;

    m_kioJob->putOnHold();
}

static bool isStatusCodeSuccess(const QNetworkReply* reply)
{
    bool ok = false;
    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&ok);
    if (!ok || statusCode < 200 || statusCode > 299)
        return false;
    return true;
}

void AccessManagerReply::readHttpResponseHeaders(KIO::Job *job)
{
    if (!job || m_metaDataRead)
        return;

    const KIO::MetaData& metaData = job->metaData();
    if (metaData.isEmpty()) {
        // Allow handling of local resources such as man pages and file url...
        if (accessManager_isLocalRequest(url())) {
            setHeader(QNetworkRequest::ContentLengthHeader, job->totalAmount(KJob::Bytes));
            setAttribute(QNetworkRequest::HttpStatusCodeAttribute, "200");
            emit metaDataChanged();
        }
        return;
    }

    // Set the encryption attribute and values...
    QSslConfiguration sslConfig;
    const bool isEncrypted = KIO::Integration::sslConfigFromMetaData(metaData, sslConfig);
    setAttribute(QNetworkRequest::ConnectionEncryptedAttribute, isEncrypted);
    if (isEncrypted)
        setSslConfiguration(sslConfig);

    // Set the returned meta data as attribute...
    setAttribute(static_cast<QNetworkRequest::Attribute>(KIO::AccessManager::MetaData),
                 metaData.toVariant());

    // Set the raw header information...
    const QStringList httpHeaders (metaData.value(QL1S("HTTP-Headers")).split(QL1C('\n'), QString::SkipEmptyParts));
    Q_FOREACH(const QString& httpHeader, httpHeaders) {
        int index = httpHeader.indexOf(QL1C(':'));
        // Handle HTTP status line...
        if (index == -1) {
            // Except for the status line, all HTTP header must be an nvpair of
            // type "<name>:<value>"
            if (!httpHeader.startsWith(QL1S("HTTP/"), Qt::CaseInsensitive))
              continue;

            QStringList statusLineAttrs (httpHeader.split(QL1C(' '), QString::SkipEmptyParts));
            if (statusLineAttrs.count() > 1)
                setAttribute(QNetworkRequest::HttpStatusCodeAttribute, statusLineAttrs.at(1));

            if (statusLineAttrs.count() > 2)
                setAttribute(QNetworkRequest::HttpReasonPhraseAttribute, statusLineAttrs.at(2));

            continue;
        }

        const QString headerName = httpHeader.left(index);
        QString headerValue = httpHeader.mid(index+1);

        // Skip setting cookies since they are automatically handled by kio_http...
        if (headerName.startsWith(QL1S("set-cookie"), Qt::CaseInsensitive))
            continue;

        if (headerName.startsWith(QL1S("content-disposition"), Qt::CaseInsensitive) &&
            (m_ignoreContentDisposition || !isStatusCodeSuccess(this)))
            continue;

        // Without overridding the corrected mime-type sent by kio_http, add
        // back the "charset=" portion of the content-type header if present.
        if (headerName.startsWith(QL1S("content-type"), Qt::CaseInsensitive)) {
            const QString mimeType = header(QNetworkRequest::ContentTypeHeader).toString();
            if (!headerValue.contains(mimeType, Qt::CaseInsensitive)) {
                index = headerValue.indexOf(QL1C(';'));
                if (index == -1)
                    headerValue = mimeType;
                else
                    headerValue.replace(0, index, mimeType);
                //kDebug(7044) << "Changed mime-type from" << mimeType << "to" << headerValue;
            }
        }

        setRawHeader(headerName.trimmed().toUtf8(), headerValue.trimmed().toUtf8());
    }

    m_metaDataRead = true;
    emit metaDataChanged();
}

int AccessManagerReply::jobError(KJob* kJob)
{
    const int errCode = kJob->error();
    switch (errCode)
    {
        case 0:
            setError(QNetworkReply::NoError, kJob->errorText());
            //kDebug(7044) << "0 -> QNetworkReply::NoError";
            break;
        case KIO::ERR_COULD_NOT_CONNECT:
            setError(QNetworkReply::ConnectionRefusedError, kJob->errorText());
            kDebug(7044) << "KIO::ERR_COULD_NOT_CONNECT -> KIO::ERR_COULD_NOT_CONNECT";
            break;
        case KIO::ERR_UNKNOWN_HOST:
            setError(QNetworkReply::HostNotFoundError, kJob->errorText());
            kDebug(7044) << "KIO::ERR_UNKNOWN_HOST -> QNetworkReply::HostNotFoundError";
            break;
        case KIO::ERR_SERVER_TIMEOUT:
            setError(QNetworkReply::TimeoutError, kJob->errorText());
            kDebug(7044) << "KIO::ERR_SERVER_TIMEOUT -> QNetworkReply::TimeoutError";
            break;
        case KIO::ERR_USER_CANCELED:
        case KIO::ERR_ABORTED:
            setError(QNetworkReply::OperationCanceledError, kJob->errorText());
            kDebug(7044) << "KIO::ERR_ABORTED -> QNetworkReply::OperationCanceledError";
            break;
        case KIO::ERR_UNKNOWN_PROXY_HOST:
            setError(QNetworkReply::ProxyNotFoundError, kJob->errorText());
            kDebug(7044) << "KIO::UNKNOWN_PROXY_HOST -> QNetworkReply::ProxyNotFoundError";
            break;
        case KIO::ERR_ACCESS_DENIED:
            setError(QNetworkReply::ContentAccessDenied, kJob->errorText());
            kDebug(7044) << "KIO::ERR_ACCESS_DENIED -> QNetworkReply::ContentAccessDenied";
            break;
        case KIO::ERR_WRITE_ACCESS_DENIED:
            setError(QNetworkReply::ContentOperationNotPermittedError, kJob->errorText());
            kDebug(7044) << "KIO::ERR_WRITE_ACCESS_DENIED -> QNetworkReply::ContentOperationNotPermittedError";
            break;
        case KIO::ERR_NO_CONTENT:
        case KIO::ERR_DOES_NOT_EXIST:
            setError(QNetworkReply::ContentNotFoundError, kJob->errorText());
            kDebug(7044) << "KIO::ERR_DOES_NOT_EXIST -> QNetworkReply::ContentNotFoundError";
            break;
        case KIO::ERR_COULD_NOT_AUTHENTICATE:
            setError(QNetworkReply::AuthenticationRequiredError, kJob->errorText());
            kDebug(7044) << errcode;
            break;
        case KIO::ERR_UNSUPPORTED_PROTOCOL:
        case KIO::ERR_NO_SOURCE_PROTOCOL:
            setError(QNetworkReply::ProtocolUnknownError, kJob->errorText());
            kDebug(7044) << errcode;
            break;
        case KIO::ERR_UNSUPPORTED_ACTION:
            setError(QNetworkReply::ProtocolInvalidOperationError, kJob->errorText());
            kDebug(7044) << errcode;
            break;
        default:
            setError(QNetworkReply::UnknownNetworkError, kJob->errorText());
            kDebug(7044) << KIO::rawErrorDetail(errCode, QString()) << "-> QNetworkReply::UnknownNetworkError";
    }

    return errCode;
}


void AccessManagerReply::slotData(KIO::Job *kioJob, const QByteArray &data)
{
    Q_UNUSED (kioJob);
    m_data += data;
    emit readyRead();
}

void AccessManagerReply::slotMimeType(KIO::Job *kioJob, const QString &mimeType)
{
    //kDebug(7044) << kioJob << mimeType;
    setHeader(QNetworkRequest::ContentTypeHeader, mimeType.toUtf8());
    readHttpResponseHeaders(kioJob);
}

void AccessManagerReply::slotResult(KJob *kJob)
{
    const int errcode = jobError(kJob);

    const QUrl redirectUrl = attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (redirectUrl.isValid())
        readHttpResponseHeaders(m_kioJob);
    else {
        setAttribute(static_cast<QNetworkRequest::Attribute>(KIO::AccessManager::KioError), errcode);
        if (errcode)
            emit error(error());
    }

    emit finished();
}

void AccessManagerReply::slotStatResult(KJob* kJob)
{
    if (jobError(kJob)) {
        emit error (error());
        emit finished();
    }

    KIO::StatJob* statJob = qobject_cast<KIO::StatJob*>(kJob);
    Q_ASSERT(statJob);

    KIO::UDSEntry entry =  statJob->statResult();
    QString mimeType = entry.stringValue(KIO::UDSEntry::UDS_MIME_TYPE);
    if (mimeType.isEmpty() && entry.isDir())
        mimeType = QL1S("inode/directory");

    if (!mimeType.isEmpty())
        setHeader(QNetworkRequest::ContentTypeHeader, mimeType.toUtf8());

    emit finished();
}

void AccessManagerReply::slotRedirection(KIO::Job* job, const KUrl& u)
{
    Q_UNUSED(job);
    setAttribute(QNetworkRequest::RedirectionTargetAttribute, QUrl(u));
}

void AccessManagerReply::slotPercent(KJob *job, unsigned long percent)
{
    qulonglong bytes = job->totalAmount(KJob::Bytes);
    emit downloadProgress(bytes * ((double)percent / 100), bytes);
}
}

#include "accessmanagerreply_p.moc"
