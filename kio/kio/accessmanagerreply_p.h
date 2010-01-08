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

#ifndef KIO_ACCESSMANAGERREPLY_P_H
#define KIO_ACCESSMANAGERREPLY_P_H

#include <QtNetwork/QNetworkReply>

namespace KIO
{
    class Job;
    class SimpleJob;
}
class KJob;
class KUrl;

namespace KDEPrivate {

/**
 * Used for KIO::AccessManager; KDE implementation of QNetworkReply.
 *
 * @since 4.3
 * @author Urs Wolfer \<uwolfer @ kde.org\>
 */

class AccessManagerReply : public QNetworkReply
{
    Q_OBJECT
public:
    AccessManagerReply(const QNetworkAccessManager::Operation &op, const QNetworkRequest &request, KIO::SimpleJob *kioJob, QObject *parent);
    virtual ~AccessManagerReply();
    virtual qint64 bytesAvailable() const;
    virtual void abort();

public Q_SLOTS:
    void appendData(KIO::Job *kioJob, const QByteArray &data);
    void setMimeType(KIO::Job *kioJob, const QString &mimeType);
    void jobDone(KJob *kJob);

protected:
    virtual qint64 readData(char *data, qint64 maxSize);
    void readHttpResponseHeaders(KIO::Job *);

private:
    class AccessManagerReplyPrivate;
    AccessManagerReplyPrivate* const d;

    Q_PRIVATE_SLOT(d, void _k_redirection(KIO::Job *job, const KUrl &url))
    Q_PRIVATE_SLOT(d, void _k_percent(KJob *job, unsigned long percent))
};

}

#endif // KIO_ACCESSMANAGERREPLY_P_H
