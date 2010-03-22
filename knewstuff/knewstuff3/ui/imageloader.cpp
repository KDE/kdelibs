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

#include "imageloader.h"

#include <kio/job.h>
#include <kio/scheduler.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <krandom.h>
#include <kdebug.h>

#include <QtCore/QFile>

using namespace KNS3;

ImageLoader::ImageLoader(const EntryInternal& entry, EntryInternal::PreviewType type, QObject* parent)
    : QObject(parent)
    , m_entry(entry)
    , m_previewType(type)
{
}

void ImageLoader::start()
{
    KUrl url(m_entry.previewUrl(m_previewType));
    if (!url.isEmpty()) {
        m_job = KIO::get(url, KIO::NoReload, KIO::HideProgressInfo);
        connect(m_job, SIGNAL(result(KJob*)), SLOT(slotDownload(KJob*)));
        connect(m_job, SIGNAL(data(KIO::Job*, const QByteArray&)), SLOT(slotData(KIO::Job*, const QByteArray&)));
        KIO::Scheduler::scheduleJob(m_job);
    }
}

KJob* ImageLoader::job()
{
    return m_job;
}

void ImageLoader::slotData(KIO::Job *job, const QByteArray& buf)
{
    Q_UNUSED(job)
    m_buffer.append(buf);
}

void ImageLoader::slotDownload(KJob *job)
{
    if (job->error()) {
        m_buffer.clear();
        return;
    }
    QImage image;
    image.loadFromData(m_buffer);
    m_buffer.clear();

    if (m_previewType == EntryInternal::PreviewSmall1
            || m_previewType == EntryInternal::PreviewSmall2
            || m_previewType == EntryInternal::PreviewSmall3) {
        if (image.width() > PreviewWidth || image.height() > PreviewHeight) {
            // if the preview is really big, first scale fast to a smaller size, then smooth to desired size
            if (image.width() > 4 * PreviewWidth || image.height() > 4 * PreviewHeight) {
                image = image.scaled(2 * PreviewWidth, 2 * PreviewHeight, Qt::KeepAspectRatio, Qt::FastTransformation);
            }
            image = image.scaled(PreviewWidth, PreviewHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        } else if (image.width() <= PreviewWidth / 2 && image.height() <= PreviewHeight / 2) {
            // upscale tiny previews to double size
            image = image.scaled(2 * image.width(), 2 * image.height());
        }
    }

    m_entry.setPreviewImage(image, m_previewType);
    emit signalPreviewLoaded(m_entry, m_previewType);
    deleteLater();
}

#include "imageloader.moc"
