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

#include <QtCore/QDebug>
#include <QtCore/QTextStream>

#include <kstandarddirs.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <ktemporaryfile.h>
#include <ksavefile.h>

#include <qtest_kde.h>
#include "ksavefiletest.h"

QTEST_KDEMAIN_CORE( KSaveFileTest )

QString tmp(QDir::tempPath() + '/');

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
        KSaveFile saveFile;
        saveFile.setFileName(targetFile);
        QVERIFY( saveFile.open() );
        QVERIFY( !QFile::exists(targetFile) );

        QTextStream ts ( &saveFile );
        ts << "This is test data one.\n";
        ts.flush();
        QCOMPARE( saveFile.error(), QFile::NoError );
        QVERIFY( !QFile::exists(targetFile) );

        QVERIFY( saveFile.finalize() );
        QVERIFY( QFile::exists(targetFile) );

        QFile::remove(targetFile);
        QVERIFY( !QFile::exists(targetFile) );
    }

    {
        //Make sure destructor does what it is supposed to do.
        {
            KSaveFile saveFile;
            saveFile.setFileName(targetFile);
            QVERIFY( saveFile.open() );
            QVERIFY( !QFile::exists(targetFile) );
        }

        QVERIFY( QFile::exists(targetFile) );
        QFile::remove(targetFile);
        QVERIFY( !QFile::exists(targetFile) );
    }

    {
        //Test some error conditions
        KSaveFile saveFile;
        QVERIFY( !saveFile.open() ); //no filename
        saveFile.setFileName(targetFile);
        QVERIFY( saveFile.open() );
        QVERIFY( !QFile::exists(targetFile) );
        QVERIFY( !saveFile.open() ); //already open

        QVERIFY( saveFile.finalize() );
        QVERIFY( QFile::exists(targetFile) );
        QVERIFY( !saveFile.finalize() ); //already finalized

        QFile::remove(targetFile);
        QVERIFY( !QFile::exists(targetFile) );
    }

    {
        //Do it again, aborting this time
        KSaveFile saveFile ( targetFile );
        QVERIFY( saveFile.open() );
        QVERIFY( !QFile::exists(targetFile) );

        QTextStream ts ( &saveFile );
        ts << "This is test data two.\n";
        ts.flush();
        QCOMPARE( saveFile.error(), QFile::NoError );
        QVERIFY( !QFile::exists(targetFile) );

        saveFile.abort();
        QVERIFY( !QFile::exists(targetFile) );
    }

    QFile file ( targetFile );
    QVERIFY( file.open(QIODevice::WriteOnly | QIODevice::Text) );
    QVERIFY( file.setPermissions( file.permissions() | QFile::ExeUser ) );
    file.close();

    {
        //Test how it works when the file already exists
        //Also check for special permissions
        KSaveFile saveFile ( targetFile );
        QVERIFY( saveFile.open() );

        QVERIFY( QFile::exists(targetFile) );
        QFileInfo fi ( targetFile );

#ifndef Q_WS_WIN
        // Windows: qt_ntfs_permission_lookup is not set by default in
        // qfsfileengine_win.cpp, could change in future Qt versions.
        QVERIFY( fi.permission( QFile::ExeUser ) );
#endif
        QVERIFY( fi.size() == 0 );

        QTextStream ts ( &saveFile );
        ts << "This is test data three.\n";
        ts.flush();

        fi.refresh();
        QVERIFY( fi.size() == 0 );
        QVERIFY( saveFile.finalize() );

        fi.refresh();
        QVERIFY( fi.size() != 0 );
#ifndef Q_WS_WIN
        QVERIFY( fi.permission( QFile::ExeUser ) );
#endif

        QFile::remove(targetFile);
    }

    {
        QFileInfo fi ( targetFile );
        targetFile = fi.fileName();
        QDir::setCurrent(fi.path());

        //one more time, this time with relative filenames
        KSaveFile saveFile ( targetFile );
        QVERIFY( saveFile.open() );
        QVERIFY( !QFile::exists(targetFile) );

        QTextStream ts ( &saveFile );
        ts << "This is test data four.\n";
        ts.flush();
        QCOMPARE( saveFile.error(), QFile::NoError );
        QVERIFY( !QFile::exists(targetFile) );

        QVERIFY( saveFile.finalize() );
        QVERIFY( QFile::exists(targetFile) );
        QFile::remove(targetFile);
    }

}

void KSaveFileTest::test_simpleBackupFile()
{
    KTemporaryFile file;
    QVERIFY( file.open() );

    QVERIFY( KSaveFile::simpleBackupFile(file.fileName()));
    QVERIFY( QFile::exists(file.fileName() + '~'));
    QFile::remove(file.fileName() + '~');

    QVERIFY( KSaveFile::simpleBackupFile(file.fileName(), tmp) );
    QFileInfo fi ( file.fileName() );
    QVERIFY( QFile::exists(tmp + fi.fileName() + '~') );
    QFile::remove(tmp + fi.fileName() + '~');
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
            QVERIFY( KSaveFile::numberedBackupFile(fi.fileName(), tmp) );
        }

        QString fileNameTemplate = tmp + fi.fileName() + ".%1~";
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
            QVERIFY( KSaveFile::numberedBackupFile(fi.fileName(), tmp) );
        }

        QString fileNameTemplate = tmp + fi.fileName() + ".%1~";
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

        QVERIFY( KSaveFile::rcsBackupFile( f.fileName(), tmp ) );
        QVERIFY( QFile::exists(tmp + fi.fileName() + ",v" ));
        QVERIFY( KSaveFile::rcsBackupFile( f.fileName(), tmp ) );
        QVERIFY( QFile::exists(tmp + fi.fileName() + ",v" ));

        filesToRemove << tmp + fi.fileName() + ",v";
    }

    {
        KTemporaryFile f;
        QVERIFY(f.open());

        QFileInfo fi ( f.fileName() );
        QVERIFY( QDir::setCurrent(fi.absolutePath()) );

        QVERIFY( KSaveFile::rcsBackupFile( fi.fileName(), tmp ) );
        QVERIFY( QFile::exists(tmp + fi.fileName() + ",v" ));
        QVERIFY( KSaveFile::rcsBackupFile( fi.fileName(), tmp ) );
        QVERIFY( QFile::exists(tmp + fi.fileName() + ",v" ));

        filesToRemove << tmp + fi.fileName() + ",v";
    }
}

void KSaveFileTest::cleanupTestCase()
{
    foreach ( const QString &fileToRemove, filesToRemove ) {
        QFile::remove(fileToRemove);
    }
}

#include "ksavefiletest.moc"
