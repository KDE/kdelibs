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

#ifndef _KPAC_DOWNLOADER_H_
#define _KPAC_DOWNLOADER_H_

#include <qobject.h>

class KURL;
namespace KIO
{
    class Job;
};

class KPACDownloader : public QObject
{
    Q_OBJECT
public:
    KPACDownloader();
    bool download(const KURL &url);
    const QCString &data() const { return m_data; }

private slots:
    void slotData(KIO::Job *, const QByteArray &);
    void slotResult(KIO::Job *);

private:
    bool m_working;
    bool m_success;
    QCString m_data;
};

#endif

// vim: ts=4 sw=4 et
