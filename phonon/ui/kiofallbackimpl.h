/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef KIOFALLBACKIMPL_H
#define KIOFALLBACKIMPL_H

#include "../kiofallback.h"
#include "../mediaobject.h"
#include <kio/global.h>

namespace KIO
{
    class Job;
    class SimpleJob;
} // namespace KIO

namespace Phonon
{

class KioFallbackImpl : public KioFallback
{
    Q_OBJECT
    public:
        KioFallbackImpl(MediaObject *parent);
        ~KioFallbackImpl();

        void stopped();
        qint64 totalTime() const;
        void setupKioStreaming();

    public Q_SLOTS:
        void setupKioJob();
        void bytestreamNeedData();
        void bytestreamEnoughData();
        void bytestreamData(KIO::Job *, const QByteArray &);
        void bytestreamResult(KJob *);
        void bytestreamTotalSize(KJob *, qulonglong);
        void cleanupByteStream();
        void bytestreamSeekStream(qint64);
        void bytestreamFileJobOpen(KIO::Job *);
        void bytestreamSeekDone(KIO::Job *, KIO::filesize_t);

    private:
        bool m_endOfDataSent;
        bool m_seeking;
        bool m_reading;
        bool m_open;
        qint64 m_seekPosition;
        KIO::SimpleJob *m_kiojob;
};

} // namespace Phonon

#endif // KIOFALLBACKIMPL_H
