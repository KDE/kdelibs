/* This file is part of the KDE Libraries
   Copyright (c) 2001 Malte Starostik <malte.starostik@t-online.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

// $Id$

#include <kapp.h>
#include <kio/job.h>

#include "kpac_downloader.moc"

KPACDownloader::KPACDownloader()
    : QObject(),
      m_success(false)
{
}

bool KPACDownloader::download(const KURL &url)
{
    m_data = 0;
    m_working = true;
    KIO::TransferJob *job = KIO::get(url, false /* no reload */, false /* no GUI */);
    connect(job, SIGNAL(data(KIO::Job *, const QByteArray &)), SLOT(slotData(KIO::Job *, const QByteArray &)));
    connect(job, SIGNAL(result(KIO::Job *)), SLOT(slotResult(KIO::Job *)));
    while (m_working)
        kapp->processOneEvent();
    return m_success;
}

void KPACDownloader::slotData(KIO::Job *, const QByteArray &data)
{
    m_data += QCString(data.data(), data.size());
}

void KPACDownloader::slotResult(KIO::Job *job)
{
    if (!(m_success = !(job->error() || static_cast<KIO::TransferJob *>(job)->isErrorPage())))
        m_data = 0;
    m_working = false;
}

// vim: ts=4 sw=4 et
