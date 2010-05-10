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
#include <kdebug.h>

#include "config-nepomuk.h"

#include <QtCore/QDateTime>

#ifdef HAVE_NEPOMUK_WITH_SDO_0_5
#include "ndo.h"
#include "nuao.h"
#include "nfo.h"
#include "nie.h"
#include "resource.h"
#include "variant.h"
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
        kDebug() << "download started: srcUrl=" << job->srcUrl()
                 << "destUrl=" << job->destUrl()
                 << "referrer=" << m_metaData.value("referrer");
        connect(job, SIGNAL(result(KJob*)), this, SLOT(slotDownloadFinished(KJob*)));
    }

private Q_SLOTS:
    void slotDownloadFinished(KJob* job)
    {
        KIO::FileCopyJob* fileCopyJob = static_cast<KIO::FileCopyJob *>(job);
        if (job->error()) {
            kDebug() << "error during download: srcUrl=" << fileCopyJob->srcUrl()
                     << "destUrl=" << fileCopyJob->destUrl()
                     << "referrer=" << m_metaData.value("referrer");
            // TODO: test whether destUrl+".part" exists
        } else {
            kDebug() << "download finished: srcUrl=" << fileCopyJob->srcUrl()
                     << "destUrl=" << fileCopyJob->destUrl()
                     << "referrer=" << m_metaData.value("referrer");
#ifdef HAVE_NEPOMUK_WITH_SDO_0_5
            //
            // Remember where a file was downloaded from the semantic way:
            // We have two file resources:
            //   one for the source file (which in most cases is a remote file)
            //   and one for the destination file (which will be or is already indexed)
            // the latter is marked as being copied from the former
            // and then there is the download event which links to the referrer.
            //

            const KUrl srcUrl = fileCopyJob->srcUrl();
            const KUrl destUrl = fileCopyJob->destUrl();
            QUrl srcType;
            QUrl destType;
            if(srcUrl.isLocalFile()) {
                srcType = Nepomuk::Vocabulary::NFO::FileDataObject();
            }
            else {
                srcType = Nepomuk::Vocabulary::NFO::RemoteDataObject();
            }
            if(destUrl.isLocalFile()) {
                destType = Nepomuk::Vocabulary::NFO::FileDataObject();
            }
            else {
                destType = Nepomuk::Vocabulary::NFO::RemoteDataObject();
            }

            // source and dest resources
            Nepomuk::Resource srcFileRes(srcUrl, srcType);
            Nepomuk::Resource destFileRes(destUrl, destType);
            srcFileRes.setProperty(Nepomuk::Vocabulary::NIE::url(), srcUrl);
            destFileRes.setProperty(Nepomuk::Vocabulary::NIE::url(), destUrl);

            // relate src and dest
            destFileRes.setProperty(Nepomuk::Vocabulary::NDO::copiedFrom(), srcFileRes);

            // details in the download event
            Nepomuk::Resource downloadEventRes(QUrl(), Nepomuk::Vocabulary::NDO::DownloadEvent());
            downloadEventRes.addProperty(Nepomuk::Vocabulary::NUAO::involves(), destFileRes);
            downloadEventRes.addProperty(Nepomuk::Vocabulary::NUAO::start(), m_downloadJobStartTime);

            // set the referrer
            KUrl referrerUrl(m_metaData.value("referrer"));
            if(referrerUrl.isValid()) {
                // TODO: we could at this point index the referrer site via strigi
                Nepomuk::Resource referrerRes(referrerUrl, Nepomuk::Vocabulary::NFO::Website());
                downloadEventRes.addProperty(Nepomuk::Vocabulary::NDO::referrer(), referrerRes);
            }
#endif
        }
    }

private:
    QMap<QString, QString> m_metaData;
    QDateTime m_downloadJobStartTime;
};
}

#endif /* BROWSERRUN_P_H */

