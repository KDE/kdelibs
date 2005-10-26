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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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

private slots:
    void initTestCase();
    void cleanupTestCase();
    void get();
    void copyFileToSamePartition();
    void copyDirectoryToSamePartition();
    void copyFileToOtherPartition();
    void copyDirectoryToOtherPartition();
    void listRecursive();
    void moveFileToSamePartition();
    void moveDirectoryToSamePartition();
    void moveFileToOtherPartition();
    void moveSymlinkToOtherPartition();
    void moveDirectoryToOtherPartition();
    void moveFileNoPermissions();
    void moveDirectoryNoPermissions();
    void newApiPerformance();
    void calculateRemainingSeconds();

signals:
    void exitLoop();

protected slots:
    void slotEntries( KIO::Job*, const KIO::UDSEntryList& lst );
    void slotGetResult( KIO::Job* );

private:
    void enterLoop();
    QString homeTmpDir() const;
    QString otherTmpDir() const;
    void copyLocalFile( const QString& src, const QString& dest );
    void copyLocalDirectory( const QString& src, const QString& dest );
    void moveLocalFile( const QString& src, const QString& dest );
    void moveLocalDirectory( const QString& src, const QString& dest );

    int m_result;
    QByteArray m_data;
    QStringList m_names;
};

#endif
