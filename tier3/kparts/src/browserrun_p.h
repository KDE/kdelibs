/* This file is part of the KDE libraries
    Copyright (c) 2009 David Faure <faure@kde.org>
    Copyright (c) 2010 Sebastian Trueg <trueg@kde.org>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License or ( at
    your option ) version 3 or, at the discretion of KDE e.V. ( which shall
    act as a proxy as in section 14 of the GPLv3 ), any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef BROWSERRUN_P_H
#define BROWSERRUN_P_H

#include <kio/copyjob.h>
#include <QDebug>

#include <QtCore/QDateTime>

#ifdef HAVE_NEPOMUK
#include "config-nepomuk.h"
#include "../nepomuk/utils/utils.h"
#endif

namespace KParts {

/**
 * This watcher is notified when the "download" job is done,
 * so we can remember where a downloaded file comes from.
 */
class DownloadJobWatcher : public QObject
{
    Q_OBJECT
public:
    DownloadJobWatcher(KIO::FileCopyJob* job, const QMap<QString, QString> &metaData)
        : QObject(job), m_metaData(metaData), m_downloadJobStartTime(QDateTime::currentDateTime())
    {
        // qDebug() << "download started: srcUrl=" << job->srcUrl()
        //         << "destUrl=" << job->destUrl()
        //         << "referrer=" << m_metaData.value("referrer");
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotDownloadFinished(KJob*)));
    }

private Q_SLOTS:
    void slotDownloadFinished(KJob* job)
    {
#ifdef HAVE_NEPOMUK
        KIO::FileCopyJob* fileCopyJob = static_cast<KIO::FileCopyJob *>(job);
        if (job->error()) {
            // qDebug() << "error during download: srcUrl=" << fileCopyJob->srcUrl()
            //         << "destUrl=" << fileCopyJob->destUrl()
            //         << "referrer=" << m_metaData.value("referrer");
            // TODO: test whether destUrl+".part" exists
        } else {
            // qDebug() << "download finished: srcUrl=" << fileCopyJob->srcUrl()
            //         << "destUrl=" << fileCopyJob->destUrl()
            //         << "referrer=" << m_metaData.value("referrer");
            Nepomuk::Utils::createCopyEvent( fileCopyJob->srcUrl(),
                                             fileCopyJob->destUrl(),
                                             m_downloadJobStartTime,
                                             QUrl(m_metaData.value("referrer")) );
        }
#else
        Q_UNUSED(job);
#endif
    }

private:
    QMap<QString, QString> m_metaData;
    QDateTime m_downloadJobStartTime;
};
}

#endif /* BROWSERRUN_P_H */

