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

#include <QFile>
#include <QTextStream>

#include <ktemporaryfile.h>
#include <kautosavefile.h>

#include <qtest_kde.h>
#include <kautosavefiletest.h>

QTEST_KDEMAIN( KAutoSaveFileTest, NoGUI )

void KAutoSaveFileTest::test_fileStaleFiles()
{ 
qDebug() << "test_fileStaleFiles";
QVERIFY(1 == 1); }

void KAutoSaveFileTest::test_applicationStaleFiles()
{
qDebug() << "test_applicationStaleFiles";

 QVERIFY(1 == 1); }

void KAutoSaveFileTest::test_locking()
{
qDebug() << "test_lcok";

 QVERIFY(1 == 1); }

void KAutoSaveFileTest::cleanupTestCase()
{
    foreach ( QString fileToRemove, filesToRemove )
    {
        QFile::remove
            (fileToRemove);
    }
}

void KAutoSaveFileTest::test_readWrite()
{
    KTemporaryFile file;
    file.setPrefix("test");

    QVERIFY( file.open() );

    KUrl normalFile( file.fileName() );


    //Test basic functionality
    KAutoSaveFile saveFile(normalFile);

    QVERIFY( !QFile::exists(saveFile.fileName()) );
    QVERIFY( saveFile.open(QIODevice::ReadWrite) );
    QVERIFY( QFile::exists(saveFile.fileName()) );

    QString inText = "This is test data one.\n";

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


    filesToRemove << file.fileName() << saveFile.fileName();
}

#include "kautosavefiletest.moc"
