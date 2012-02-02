/*  This file is part of the KDE libraries
    Copyright (c) 2006 Jacob R Rideout <kde@jacobrideout.net>

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

#include "kautosavefiletest.h"

#include <QtCore/QFile>
#include <QtCore/QTextStream>

#include <qtemporaryfile.h>
#include <kautosavefile.h>

#include <QtTest/QtTest>

QTEST_MAIN( KAutoSaveFileTest )

void KAutoSaveFileTest::initTestCase()
{
    QCoreApplication::instance()->setApplicationName(QLatin1String("qttest")); // TODO do this in qtestlib itself
}

void KAutoSaveFileTest::cleanupTestCase()
{
    Q_FOREACH (const QString &fileToRemove, filesToRemove) {
        QFile::remove(fileToRemove);
    }
}

void KAutoSaveFileTest::test_readWrite()
{
    QTemporaryFile file;

    QVERIFY( file.open() );

    QUrl normalFile( QFileInfo(file).absoluteFilePath() );

    //Test basic functionality
    KAutoSaveFile saveFile(normalFile);

    QVERIFY( !QFile::exists(saveFile.fileName()) );
    QVERIFY( saveFile.open(QIODevice::ReadWrite) );

    QString inText = QString::fromLatin1("This is test data one.\n");

    {
        QTextStream ts ( &saveFile );
        ts << inText;
        ts.flush();
    }

    saveFile.close();

    {
        QFile testReader(saveFile.fileName());
        testReader.open(QIODevice::ReadWrite);
        QTextStream ts ( &testReader );

        QString outText = ts.readAll();

        QCOMPARE( outText, inText );
    }


    filesToRemove << file.fileName();
}

void KAutoSaveFileTest::test_fileStaleFiles()
{
    // TODO
}

void KAutoSaveFileTest::test_applicationStaleFiles()
{
    // TODO
}

void KAutoSaveFileTest::test_locking()
{
    QUrl normalFile( QString::fromLatin1("fish://user@example.com/home/remote/test.txt") );

    KAutoSaveFile saveFile(normalFile);

    QVERIFY( !QFile::exists(saveFile.fileName()) );
    QVERIFY( saveFile.open(QIODevice::ReadWrite) );

    const QList<KAutoSaveFile *> staleFiles( KAutoSaveFile::staleFiles( normalFile ) );

    QVERIFY( !staleFiles.isEmpty() );

    KAutoSaveFile* saveFile2 = staleFiles.at(0);

    const QString fn = saveFile2->fileName();
    // It looks like $XDG_DATA_HOME/stalefiles/qttest/test.txtXXXfish_%2Fhome%2FremoteXXXXXXX
    QVERIFY2( fn.contains(QLatin1String("stalefiles/qttest/test.txt")), qPrintable(fn) );
    QVERIFY2( fn.contains(QLatin1String("fish_%2Fhome%2Fremote")), qPrintable(fn) );

    QVERIFY( QFile::exists(saveFile2->fileName()) );
    QVERIFY( !saveFile2->open(QIODevice::ReadWrite) );

    saveFile.releaseLock();

    QVERIFY( saveFile2->open(QIODevice::ReadWrite) );

    delete saveFile2;

}
