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

#ifndef KNEWSTUFF3_IMAGELOADER_H
#define KNEWSTUFF3_IMAGELOADER_H

#include <QtGui/QImage>
#include <QtCore/QObject>
#include <QtCore/QByteArray>

#include <kdebug.h>

#include "core/entryinternal.h"

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
class ImageLoader : public QObject
{
    Q_OBJECT
public:
    ImageLoader(const EntryInternal& entry, EntryInternal::PreviewType type, QObject* parent);
    void start();
    /**
     * Get the job doing the image loading in the background (to have progress information available)
     * @return the job
     */
    KJob* job();

Q_SIGNALS:
    void signalPreviewLoaded(const KNS3::EntryInternal&, KNS3::EntryInternal::PreviewType);

private Q_SLOTS:
    void slotDownload(KJob *job);
    void slotData(KIO::Job* job, const QByteArray& buf);

private:
    EntryInternal m_entry;
    EntryInternal::PreviewType m_previewType;
    QByteArray m_buffer;
    KIO::TransferJob *m_job;
};
}
#endif

