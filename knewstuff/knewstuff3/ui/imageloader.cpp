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

ImageLoader::ImageLoader(const QString& url, QObject* parent)
        : QObject(parent), QImage(), m_url(url)
{
    if (!m_url.isEmpty()) {
        m_job = KIO::get(m_url, KIO::NoReload, KIO::HideProgressInfo);
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
    loadFromData(m_buffer);
    m_buffer.clear();
    emit signalLoaded(m_url, *this);
}

#include "imageloader.moc"
