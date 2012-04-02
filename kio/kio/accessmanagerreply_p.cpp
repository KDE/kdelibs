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
#include "scheduler.h"

#include <kdebug.h>
#include <kauthorized.h>
#include <kprotocolinfo.h>
#include <qmimedatabase.h>

#include <QtNetwork/QSslConfiguration>

#define QL1S(x)  QLatin1String(x)
#define QL1C(x)  QLatin1Char(x)

namespace KDEPrivate {

AccessManagerReply::AccessManagerReply(const QNetworkAccessManager::Operation &op,
                                       const QNetworkRequest &request,
                                       KIO::SimpleJob *kioJob,
                                       bool emitReadyReadOnMetaDataChange,
                                       QObject *parent)
                   :QNetworkReply(parent),
                    m_metaDataRead(false),
                    m_ignoreContentDisposition(false),
                    m_emitReadyReadOnMetaDataChange(emitReadyReadOnMetaDataChange),
                    m_kioJob(kioJob)

{
    setRequest(request);
    setOpenMode(QIODevice::ReadOnly);
    setUrl(request.url());
    setOperation(op);
    setError(NoError, QString());

    if (!request.sslConfiguration().isNull())
        setSslConfiguration(request.sslConfiguration());

    if (kioJob) {
        connect(kioJob, SIGNAL(redirection(KIO::Job*,KUrl)), SLOT(slotRedirection(KIO::Job*,KUrl)));
        connect(kioJob, SIGNAL(percent(KJob*,ulong)), SLOT(slotPercent(KJob*,ulong)));

        if (qobject_cast<KIO::StatJob*>(kioJob)) {
            connect(kioJob, SIGNAL(result(KJob*)), SLOT(slotStatResult(KJob*)));
        } else {
            connect(kioJob, SIGNAL(result(KJob*)), SLOT(slotResult(KJob*)));
            connect(kioJob, SIGNAL(data(KIO::Job*,QByteArray)),
                SLOT(slotData(KIO::Job*,QByteArray)));
            connect(kioJob, SIGNAL(mimetype(KIO::Job*,QString)),
                SLOT(slotMimeType(KIO::Job*,QString)));
        }
    }
}

AccessManagerReply::~AccessManagerReply()
{
}

void AccessManagerReply::abort()
{
    if (m_kioJob) {
        m_kioJob.data()->kill();
        m_kioJob.clear();
    }

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

bool AccessManagerReply::ignoreContentDisposition (KIO::Job* job)
{
    if (m_ignoreContentDisposition) {
        return true;
    }

    if (job->queryMetaData(QL1S("content-disposition-type")).isEmpty()) {
        return true;
    }

    bool ok = false;
    const int statusCode = attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&ok);
    if (!ok || statusCode < 200 || statusCode > 299) {
        return true;
    }

    return false;
}

void AccessManagerReply::setIgnoreContentDisposition(bool on)
{
    // kDebug(7044) << on;
    m_ignoreContentDisposition = on;
}

void AccessManagerReply::setStatus(const QString& message, QNetworkReply::NetworkError code)
{
    setError(code, message);
    if (code != QNetworkReply::NoError) {
        QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection, Q_ARG(QNetworkReply::NetworkError, code));
    }
    QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
}

void AccessManagerReply::putOnHold()
{
    if (!m_kioJob || isFinished())
        return;

    // kDebug(7044) << m_kioJob << m_data;
    m_kioJob.data()->disconnect(this);
    m_kioJob.data()->putOnHold();
    m_kioJob.clear();
    KIO::Scheduler::publishSlaveOnHold();
}

bool AccessManagerReply::isLocalRequest(const QUrl& url)
{
    const QString scheme(url.scheme());
    return (KProtocolInfo::isKnownProtocol(scheme) &&
            KProtocolInfo::protocolClass(scheme).compare(QL1S(":local"), Qt::CaseInsensitive) == 0);
}

void AccessManagerReply::readHttpResponseHeaders(KIO::Job *job)
{
    if (!job || m_metaDataRead)
        return;

    KIO::MetaData metaData (job->metaData());
    if (metaData.isEmpty()) {
        // Allow handling of local resources such as man pages and file url...
        if (isLocalRequest(url())) {
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

    // Set the raw header information...
    const QStringList httpHeaders (metaData.value(QL1S("HTTP-Headers")).split(QL1C('\n'), QString::SkipEmptyParts));
    if (httpHeaders.isEmpty()) {
      if (metaData.contains(QL1S("charset"))) {
         QString mimeType = header(QNetworkRequest::ContentTypeHeader).toString();
         mimeType += QL1S(" ; charset=");
         mimeType += metaData.value(QL1S("charset"));
         kDebug(7044) << "changed content-type to" << mimeType;
         setHeader(QNetworkRequest::ContentTypeHeader, mimeType.toUtf8());
      }
    } else {
        Q_FOREACH(const QString& httpHeader, httpHeaders) {
            int index = httpHeader.indexOf(QL1C(':'));
            // Handle HTTP status line...
            if (index == -1) {
                // Except for the status line, all HTTP header must be an nvpair of
                // type "<name>:<value>"
                if (!httpHeader.startsWith(QL1S("HTTP/"), Qt::CaseInsensitive)) {
                    continue;
                }

                QStringList statusLineAttrs (httpHeader.split(QL1C(' '), QString::SkipEmptyParts));
                if (statusLineAttrs.count() > 1) {
                    setAttribute(QNetworkRequest::HttpStatusCodeAttribute, statusLineAttrs.at(1));
                }

                if (statusLineAttrs.count() > 2) {
                    setAttribute(QNetworkRequest::HttpReasonPhraseAttribute, statusLineAttrs.at(2));
                }

                continue;
            }

            const QString headerName = httpHeader.left(index);
            QString headerValue = httpHeader.mid(index+1);

            // Ignore cookie header since it is handled by the http ioslave.
            if (headerName.startsWith(QL1S("set-cookie"), Qt::CaseInsensitive)) {
                continue;
            }

            if (headerName.startsWith(QL1S("content-disposition"), Qt::CaseInsensitive) &&
                ignoreContentDisposition(job)) {
                continue;
            }

            // Without overridding the corrected mime-type sent by kio_http, add
            // back the "charset=" portion of the content-type header if present.
            if (headerName.startsWith(QL1S("content-type"), Qt::CaseInsensitive)) {

                QString mimeType (header(QNetworkRequest::ContentTypeHeader).toString());

                if (m_ignoreContentDisposition) {
                    // If the server returned application/octet-stream, try to determine the
                    // real content type from the disposition filename.
                    if (mimeType == QL1S("application/octet-stream")) {
                        const QString fileName (metaData.value(QL1S("content-disposition-filename")));
                        QMimeDatabase db;
                        QMimeType mime = db.mimeTypeForFile((fileName.isEmpty() ? url().path() : fileName), QMimeDatabase::MatchExtension);
                        mimeType = mime.name();
                    }
                    metaData.remove(QL1S("content-disposition-type"));
                    metaData.remove(QL1S("content-disposition-filename"));
                }

                if (!headerValue.contains(mimeType, Qt::CaseInsensitive)) {
                    index = headerValue.indexOf(QL1C(';'));
                    if (index == -1) {
                        headerValue = mimeType;
                    } else {
                        headerValue.replace(0, index, mimeType);
                    }
                    //kDebug(7044) << "Changed mime-type from" << mimeType << "to" << headerValue;
                }
            }
            setRawHeader(headerName.trimmed().toUtf8(), headerValue.trimmed().toUtf8());
        }
    }

    // Set the returned meta data as attribute...
    setAttribute(static_cast<QNetworkRequest::Attribute>(KIO::AccessManager::MetaData), metaData.toVariant());

    m_metaDataRead = true;
    emit metaDataChanged();
}

int AccessManagerReply::jobError(KJob* kJob)
{
    const int errCode = kJob->error();
    switch (errCode)
    {
        case 0:
            break; // No error;
        case KIO::ERR_SLAVE_DEFINED:
        case KIO::ERR_NO_CONTENT: // Sent by a 204 response is not an error condition.
            setError(QNetworkReply::NoError, kJob->errorText());
            //kDebug(7044) << "0 -> QNetworkReply::NoError";
            break;
        case KIO::ERR_IS_DIRECTORY:
            // This error condition can happen if you click on an ftp link that points
            // to a directory instead of a file, e.g. ftp://ftp.kde.org/pub
            setHeader(QNetworkRequest::ContentTypeHeader, "inode/directory");
            setError(QNetworkReply::NoError, kJob->errorText());
            break;
        case KIO::ERR_COULD_NOT_CONNECT:
            setError(QNetworkReply::ConnectionRefusedError, kJob->errorText());
            kDebug(7044) << "KIO::ERR_COULD_NOT_CONNECT -> QNetworkReply::ConnectionRefusedError";
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
        case KIO::ERR_DOES_NOT_EXIST:
            setError(QNetworkReply::ContentNotFoundError, kJob->errorText());
            kDebug(7044) << "KIO::ERR_DOES_NOT_EXIST -> QNetworkReply::ContentNotFoundError";
            break;
        case KIO::ERR_COULD_NOT_AUTHENTICATE:
            setError(QNetworkReply::AuthenticationRequiredError, kJob->errorText());
            kDebug(7044) << "KIO::ERR_COULD_NOT_AUTHENTICATE -> QNetworkReply::AuthenticationRequiredError";
            break;
        case KIO::ERR_UNSUPPORTED_PROTOCOL:
        case KIO::ERR_NO_SOURCE_PROTOCOL:
            setError(QNetworkReply::ProtocolUnknownError, kJob->errorText());
            kDebug(7044) << "KIO::ERR_UNSUPPORTED_PROTOCOL -> QNetworkReply::ProtocolUnknownError";
            break;
        case KIO::ERR_CONNECTION_BROKEN:
            setError(QNetworkReply::RemoteHostClosedError, kJob->errorText());
            kDebug(7044) << "KIO::ERR_CONNECTION_BROKEN -> QNetworkReply::RemoteHostClosedError";
            break;
        case KIO::ERR_UNSUPPORTED_ACTION:
            setError(QNetworkReply::ProtocolInvalidOperationError, kJob->errorText());
            kDebug(7044) << "KIO::ERR_UNSUPPORTED_ACTION -> QNetworkReply::ProtocolInvalidOperationError";
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
    if (!data.isEmpty())
        emit readyRead();
}

void AccessManagerReply::slotMimeType(KIO::Job *kioJob, const QString &mimeType)
{
    //kDebug(7044) << kioJob << mimeType;
    setHeader(QNetworkRequest::ContentTypeHeader, mimeType.toUtf8());
    readHttpResponseHeaders(kioJob);
    if (m_emitReadyReadOnMetaDataChange) {
        emit readyRead();
    }
}

void AccessManagerReply::slotResult(KJob *kJob)
{
    const int errcode = jobError(kJob);

    const QUrl redirectUrl = attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (!redirectUrl.isValid()) {
        setAttribute(static_cast<QNetworkRequest::Attribute>(KIO::AccessManager::KioError), errcode);
        if (errcode && errcode != KIO::ERR_NO_CONTENT)
            emit error(error());
    }

    // Make sure HTTP response headers are always set.
    if (!m_metaDataRead) {
        readHttpResponseHeaders(qobject_cast<KIO::Job*>(kJob));
    }

    emit finished();
}

void AccessManagerReply::slotStatResult(KJob* kJob)
{
    if (jobError(kJob)) {
        emit error (error());
        emit finished();
        return;
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

    if (!KAuthorized::authorizeUrlAction(QLatin1String("redirect"), url(), u)) {
        kWarning(7007) << "Redirection from" << url() << "to" << u << "REJECTED by policy!";
        setError(QNetworkReply::ContentAccessDenied, u.url());
        emit error(error());
        return;
    }
    setAttribute(QNetworkRequest::RedirectionTargetAttribute, QUrl(u));
}

void AccessManagerReply::slotPercent(KJob *job, unsigned long percent)
{
    qulonglong bytesTotal = job->totalAmount(KJob::Bytes);
    qulonglong bytesProcessed = bytesTotal * (percent / 100);
    if (operation() == QNetworkAccessManager::PutOperation ||
        operation() == QNetworkAccessManager::PostOperation) {
        emit uploadProgress(bytesProcessed, bytesTotal);
        return;
    }
    emit downloadProgress(bytesProcessed, bytesTotal);
}
}

