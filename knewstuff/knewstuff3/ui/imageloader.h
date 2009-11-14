/*
    This file is part of KNewStuff2.
    Copyright (c) 2006, 2007 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KNEWSTUFF2_UI_QASYNCPIXMAP_H
#define KNEWSTUFF2_UI_QASYNCPIXMAP_H

#include <QtGui/QImage>
#include <QtCore/QObject>
#include <QtCore/QByteArray>

class KJob;
namespace KIO
{
    class Job;
    class TransferJob;
}

namespace KNS3 {

/**
 * Convenience class for images with remote sources.
 *
 * This class represents a fire-and-forget approach of loading images
 * in applications. The image will load itself.
 * Using this class also requires using QAsyncFrame or similar UI
 * elements which allow for asynchronous image loading.
 *
 * This class is used internally by the DownloadDialog class.
 *
 * @internal
 */
class ImageLoader : public QObject, public QImage
{
    Q_OBJECT
public:
    ImageLoader(const QString& url, QObject* parent);
    
    /**
     * Get the job doing the image loading in the background (to have progress information available)
     * @return the job
     */
    KJob* job();
    
Q_SIGNALS:
    void signalLoaded(const QString & url, const QImage& pix);

private Q_SLOTS:
    void slotDownload(KJob *job);
    void slotData(KIO::Job* job, const QByteArray& buf);

private:
    QString m_url;
    QByteArray m_buffer;
    KIO::TransferJob *m_job;
};
}
#endif

