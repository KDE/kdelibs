/* This file is part of the KDE project
   Copyright (C) 2004 David Faure <faure@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef JOBTEST_H
#define JOBTEST_H

#include <qstring.h>
#include <qobject.h>
#include <kio/job.h>

class JobTest : public QObject
{
    Q_OBJECT

public:
    JobTest() {}
    void setup();
    void runAll();
    void cleanup();

    void copyFileToSamePartition();
    void copyDirectoryToSamePartition();
    void copyFileToOtherPartition();
    void copyDirectoryToOtherPartition();
    void listRecursive();

private slots:
    void slotEntries( KIO::Job*, const KIO::UDSEntryList& lst );

private:
    QString homeTmpDir() const;
    QString otherTmpDir() const;
    void copyLocalFile( const QString& src, const QString& dest );
    void copyLocalDirectory( const QString& src, const QString& dest );

    QStringList m_names;
};

#endif
