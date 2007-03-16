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

#ifndef KIOFALLBACK_H
#define KIOFALLBACK_H

#include "phononnamespace.h"

#include <kio/global.h>

class QByteArray;
class KJob;

namespace KIO
{
    class Job;
    class SimpleJob;
} // namespace KIO

namespace Phonon
{
class MediaObject;

class KioFallback : public QObject
{
    Q_OBJECT
    public:
        KioFallback(MediaObject *parent);
        ~KioFallback();

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
        bool endOfDataSent;
        bool seeking;
        bool reading;
        KIO::SimpleJob *kiojob;
};
} // namespace Phonon
#endif // KIOFALLBACK_H
