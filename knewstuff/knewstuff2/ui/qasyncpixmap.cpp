/*
    This file is part of KNewStuff2.
    Copyright (c) 2006, 2007 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "qasyncpixmap.h"

#include <kio/job.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <krandom.h>
#include <kdebug.h>

#include <QtCore/QFile>

QAsyncPixmap::QAsyncPixmap(const QString& url, QObject* parent)
    : QObject(parent), QPixmap(), m_url(url)
{
    if (!m_url.isEmpty()) {
        // XXX ???
         //KTempFile
        m_dest = KGlobal::dirs()->saveLocation("tmp") + KRandom::randomString(10) + ".png";

        KIO::FileCopyJob *job = KIO::file_copy(m_url, m_dest, -1, KIO::Overwrite | KIO::HideProgressInfo);
        connect(job, SIGNAL(result(KJob*)), SLOT(slotDownload(KJob*)));
    }
}

void QAsyncPixmap::slotDownload(KJob *job)
{
    //kDebug(550) << "DOWNLOAD";
    if(job->error())
    {
        // XXX ???
        return;
    }
    load(m_dest);
    //bool ret = load(m_dest);
//    QFile::remove(m_dest);
    //kDebug(550) << "DOWNLOADed to " << m_dest;
    //kDebug(550) << "ret = " << ret;

    emit signalLoaded(m_url, *this);
}

#include "qasyncpixmap.moc"
