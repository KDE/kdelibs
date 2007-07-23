/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       David Faure <faure@kde.org>
                       Waldo Bastian <bastian@kde.org>
    Copyright (C) 2007 Thiago Macieira <thiago@kde.org>

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

#ifndef KIO_JOB_P_H
#define KIO_JOB_P_H

#include "job.h"
#include "kcompositejob_p.h"

/** @internal */
class KIO::JobPrivate: public KCompositeJobPrivate
{
public:
    JobPrivate() : m_parentJob( 0L ), m_extraFlags(0)
                   {}

    // Maybe we could use the QObject parent/child mechanism instead
    // (requires a new ctor, and moving the ctor code to some init()).
    Job* m_parentJob;
    int m_extraFlags;

    Q_DECLARE_PUBLIC(Job)
};

class KIO::SimpleJobPrivate: public KIO::JobPrivate
{
public:
    void simpleJobInit();

    Q_DECLARE_PUBLIC(SimpleJob)
};

class KIO::TransferJobPrivate: public KIO::SimpleJobPrivate
{
public:
    Q_DECLARE_PUBLIC(TransferJob)
};

#endif
