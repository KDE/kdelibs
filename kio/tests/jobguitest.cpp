/* This file is part of the KDE project
   Copyright (C) 2011 David Faure <faure@kde.org>

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

#include "qtest_kde.h"
#include <kio/copyjob.h>
#include <kio/netaccess.h>
#include <kio/paste.h>
#include <kio/deletejob.h>
#include "kiotesthelper.h" // createTestFile etc.

#include <QtGui/QClipboard>

static QString otherTmpDir()
{
#ifdef Q_WS_WIN
    return QDir::tempPath() + "/jobtest/";
#else
    // This one needs to be on another partition
    return "/tmp/jobtest/";
#endif
}

class JobGuiTest : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void initTestCase()
    {
        // Start with a clean base dir
        cleanupTestCase();
        homeTmpDir(); // create it
        if ( !QFile::exists( otherTmpDir() ) ) {
            bool ok = QDir().mkdir( otherTmpDir() );
            if ( !ok )
                kFatal() << "Couldn't create " << otherTmpDir();
        }
    }

    void cleanupTestCase()
    {
        delDir( homeTmpDir() );
        delDir( otherTmpDir() );
    }

    void pasteFileToOtherPartition()
    {
        const QString filePath = homeTmpDir() + "fileFromHome";
        const QString dest = otherTmpDir() + "fileFromHome_copied";
        QFile::remove(dest);
        createTestFile( filePath );

        QMimeData* mimeData = new QMimeData;
        KUrl fileUrl(filePath);
        fileUrl.populateMimeData(mimeData);
        QApplication::clipboard()->setMimeData(mimeData);

        KIO::Job* job = KIO::pasteClipboard(otherTmpDir(), static_cast<QWidget*>(0));
        job->setUiDelegate(0);
        bool ok = KIO::NetAccess::synchronousRun(job, 0);
        QVERIFY( ok );

        QVERIFY( QFile::exists( dest ) );
        QVERIFY( QFile::exists( filePath ) ); // still there
    }

private:
    static void delDir(const QString& pathOrUrl) {
        KIO::Job* job = KIO::del(KUrl(pathOrUrl), KIO::HideProgressInfo);
        job->setUiDelegate(0);
        KIO::NetAccess::synchronousRun(job, 0);
    }


};

QTEST_KDEMAIN( JobGuiTest, GUI )

#include "jobguitest.moc"
