/* kate: tab-indents off; replace-tabs on; tab-width 4; remove-trailing-space on; encoding utf-8;*/
/*
  This file is part of the KDE libraries
  Copyright 2006 Allen Winter <winter@kde.org>
  Copyright 2006 Jaison Lee <lee.jaison@gmail.com>

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

#include <QDebug>
#include <QTextStream>

#include <kapplication.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <ktemporaryfile.h>
#include <ksavefile.h>

#include <qtest_kde.h>
#include <ksavefiletest.h>

QTEST_KDEMAIN( KSaveFileTest, NoGUI )

void KSaveFileTest::test_ksavefile()
{
    QString targetFile;

    {
        //This will be the file we eventually write to. Yes, I know you
        //should never remove the temporaryfile and then expect the filename
        //to continue to be unique, but this is a test for crying out loud. :)
        KTemporaryFile file;
        file.setPrefix("ksavefiletest");
        QVERIFY( file.open() );
        targetFile = file.fileName();
    }

    {
        //Test basic functionality
        KSaveFile saveFile ( targetFile );
        QCOMPARE( saveFile.status(), 0 );
        QVERIFY( !QFile::exists(targetFile) );

        QTextStream *ts = saveFile.textStream();
        (*ts) << "Hello out there in TV land!\n";
        ts->flush();
        QCOMPARE( saveFile.status(), 0 );
        QVERIFY( !QFile::exists(targetFile) );

        QVERIFY( saveFile.close() );
        QVERIFY( QFile::exists(targetFile) );
    }

    QFile::remove(targetFile);
    QVERIFY( !QFile::exists(targetFile) );

    {
        //Do it again, aborting this time
        KSaveFile saveFile ( targetFile );
        QCOMPARE( saveFile.status(), 0 );
        QVERIFY( !QFile::exists(targetFile) );

        QTextStream *ts = saveFile.textStream();
        (*ts) << "Hello out there in TV land!\n";
        ts->flush();
        QCOMPARE( saveFile.status(), 0 );
        QVERIFY( !QFile::exists(targetFile) );

        saveFile.abort();
        QVERIFY( !QFile::exists(targetFile) );
    }

    QFileInfo fi ( targetFile );
    targetFile = fi.fileName();
    QDir::setCurrent(fi.path());

    {
        //one last time, this time with relative filenames
        KSaveFile saveFile ( targetFile );
        QCOMPARE( saveFile.status(), 0 );
        QVERIFY( !QFile::exists(targetFile) );

        QTextStream *ts = saveFile.textStream();
        (*ts) << "Hello out there in TV land!\n";
        ts->flush();
        QCOMPARE( saveFile.status(), 0 );
        QVERIFY( !QFile::exists(targetFile) );

        QVERIFY( saveFile.close() );
        QVERIFY( QFile::exists(targetFile) );
    }
}

void KSaveFileTest::test_simpleBackupFile()
{
    KTemporaryFile file;
    QVERIFY( file.open() );

    QVERIFY( KSaveFile::simpleBackupFile(file.fileName()));
    QVERIFY( QFile::exists(file.fileName() + '~'));
    QFile::remove(file.fileName() + '~');

    QVERIFY( KSaveFile::simpleBackupFile(file.fileName(), "/tmp/") );
    QFileInfo fi ( file.fileName() );
    QVERIFY( QFile::exists("/tmp/" + fi.fileName() + '~') );
    QFile::remove("/tmp/" + fi.fileName() + '~');
}

void KSaveFileTest::test_numberedBackupFile()
{
    // Test absolute pathname file
    {
        KTemporaryFile file;
        QVERIFY( file.open() );

        for ( int i=1; i<15; i++ ) {
            QVERIFY( KSaveFile::numberedBackupFile(file.fileName()) );
        }

        QString fileNameTemplate = file.fileName() + ".%1~";
        for ( int i=1; i<=10; i++ ) {
            QVERIFY( QFile::exists(fileNameTemplate.arg(i)) );
            filesToRemove << fileNameTemplate.arg(i);
        }

        QVERIFY( !QFile::exists(fileNameTemplate.arg(11)) );
        QVERIFY( !QFile::exists(fileNameTemplate.arg(12)) );
        QVERIFY( !QFile::exists(fileNameTemplate.arg(13)) );
        QVERIFY( !QFile::exists(fileNameTemplate.arg(14)) );
    }

    // Test current directory
    {
        KTemporaryFile file;
        QVERIFY( file.open() );

        QFileInfo fi ( file.fileName() );
        QVERIFY( QDir::setCurrent(fi.absolutePath()) );

        for ( int i=1; i<15; i++ ) {
            QVERIFY( KSaveFile::numberedBackupFile(fi.fileName()) );
        }

        QString fileNameTemplate = fi.fileName() + ".%1~";
        for ( int i=1; i<=10; i++ ) {
            QVERIFY( QFile::exists(fileNameTemplate.arg(i)) );
            filesToRemove << fileNameTemplate.arg(i);
        }

        QVERIFY( !QFile::exists(fileNameTemplate.arg(11)) );
        QVERIFY( !QFile::exists(fileNameTemplate.arg(12)) );
        QVERIFY( !QFile::exists(fileNameTemplate.arg(13)) );
        QVERIFY( !QFile::exists(fileNameTemplate.arg(14)) );
    }

    // Test absolute pathname file w/new directory
    {
        KTemporaryFile file;
        QVERIFY( file.open() );

        QFileInfo fi ( file.fileName() );
        QVERIFY( QDir::setCurrent(fi.absolutePath()) );

        for ( int i=1; i<15; i++ ) {
            QVERIFY( KSaveFile::numberedBackupFile(fi.fileName(), "/tmp") );
        }

        QString fileNameTemplate = "/tmp/" + fi.fileName() + ".%1~";
        for ( int i=1; i<=10; i++ ) {
            QVERIFY( QFile::exists(fileNameTemplate.arg(i)) );
            filesToRemove << fileNameTemplate.arg(i);
        }

        QVERIFY( !QFile::exists(fileNameTemplate.arg(11)) );
        QVERIFY( !QFile::exists(fileNameTemplate.arg(12)) );
        QVERIFY( !QFile::exists(fileNameTemplate.arg(13)) );
        QVERIFY( !QFile::exists(fileNameTemplate.arg(14)) );
    }

    // Test current directory w/new directory
    {
        KTemporaryFile file;
        QVERIFY(file.open());

        QFileInfo fi ( file.fileName() );
        QVERIFY( QDir::setCurrent(fi.absolutePath()) );

        for ( int i=1; i<15; i++ ) {
            QVERIFY( KSaveFile::numberedBackupFile(fi.fileName(), "/tmp") );
        }

        QString fileNameTemplate = "/tmp/" + fi.fileName() + ".%1~";
        for ( int i=1; i<=10; i++ ) {
            QVERIFY( QFile::exists(fileNameTemplate.arg(i)) );
            filesToRemove << fileNameTemplate.arg(i);
        }

        QVERIFY( !QFile::exists(fileNameTemplate.arg(11)) );
        QVERIFY( !QFile::exists(fileNameTemplate.arg(12)) );
        QVERIFY( !QFile::exists(fileNameTemplate.arg(13)) );
        QVERIFY( !QFile::exists(fileNameTemplate.arg(14)) );
    }

}

void KSaveFileTest::test_rcsBackupFile()
{
    QString cipath = KStandardDirs::findExe("ci");
    if (cipath.isEmpty())
        QSKIP("ci not available", SkipAll);

    {
        KTemporaryFile f;
        QVERIFY(f.open());

        QVERIFY( KSaveFile::rcsBackupFile( f.fileName() ) );
        QVERIFY( QFile::exists(f.fileName() + ",v" ));
        QVERIFY( KSaveFile::rcsBackupFile( f.fileName() ) );
        QVERIFY( QFile::exists(f.fileName() + ",v" ));

        filesToRemove << f.fileName() + ",v";
    }

    {
        KTemporaryFile f;
        QVERIFY(f.open());

        QVERIFY( KSaveFile::rcsBackupFile( f.fileName() ) );
        QVERIFY( QFile::exists(f.fileName() + ",v" ) );

        QTextStream out( &f );
        out << "Testing a change\n";
        out.flush();

        QVERIFY( KSaveFile::rcsBackupFile( f.fileName() ) );

        out << "Testing another change\n";
        out.flush();

        QVERIFY( KSaveFile::rcsBackupFile( f.fileName() ) );
        filesToRemove << f.fileName() + ",v";
    }

    {
        KTemporaryFile f;
        QVERIFY(f.open());

        QFileInfo fi ( f.fileName() );
        QVERIFY( QDir::setCurrent(fi.absolutePath()) );

        QVERIFY( KSaveFile::rcsBackupFile( fi.fileName() ) );
        QVERIFY( QFile::exists(f.fileName() + ",v" ));
        QVERIFY( KSaveFile::rcsBackupFile( fi.fileName() ) );
        QVERIFY( QFile::exists(f.fileName() + ",v" ));

        filesToRemove << f.fileName() + ",v";
    }

    {
        KTemporaryFile f;
        QVERIFY(f.open());

        QFileInfo fi ( f.fileName() );

        QVERIFY( KSaveFile::rcsBackupFile( f.fileName(), "/tmp/" ) );
        QVERIFY( QFile::exists("/tmp/" + fi.fileName() + ",v" ));
        QVERIFY( KSaveFile::rcsBackupFile( f.fileName(), "/tmp/" ) );
        QVERIFY( QFile::exists("/tmp/" + fi.fileName() + ",v" ));

        filesToRemove << "/tmp/" + fi.fileName() + ",v";
    }

    {
        KTemporaryFile f;
        QVERIFY(f.open());

        QFileInfo fi ( f.fileName() );
        QVERIFY( QDir::setCurrent(fi.absolutePath()) );

        QVERIFY( KSaveFile::rcsBackupFile( fi.fileName(), "/tmp/" ) );
        QVERIFY( QFile::exists("/tmp/" + fi.fileName() + ",v" ));
        QVERIFY( KSaveFile::rcsBackupFile( fi.fileName(), "/tmp/" ) );
        QVERIFY( QFile::exists("/tmp/" + fi.fileName() + ",v" ));

        filesToRemove << "/tmp/" + fi.fileName() + ",v";
    }
}


void KSaveFileTest::test_dataStream()
{
#ifdef Q_WS_WIN

    QString path=QDir::homePath();

    path = path + QLatin1String("/test_KSaveFileTest_dataStream.tmp");

    printf("KSaveFileTest::test_dataStream(): path='%s'\n", qPrintable(path));

    KSaveFile* database = new KSaveFile(path);


    // msvc linked against QtCore4.lib (release version) crashes in
    //     QFile * KTempFile::file()
    //     { ...
    //     mFile->open(mStream, QIODevice::ReadWrite);
    //
    //     mFile is QFile*
    QDataStream* m_str = database->dataStream();

    delete database;
#endif
}

void KSaveFileTest::cleanupTestCase()
{
    foreach ( QString fileToRemove, filesToRemove ) {
        QFile::remove(fileToRemove);
    }
}

#include "ksavefiletest.moc"
