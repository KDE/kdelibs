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

#include "ksavefiletest.h"

#include "kbackup.h" // TODO split out into a separate test

#include <QtCore/QDebug>
#include <QtCore/QTextStream>

#include <qstandardpaths.h>
#include <qtemporaryfile.h>

#include <kdebug.h>
#include <klocalizedstring.h>
#include <kglobal.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <ktemporaryfile.h>
#include <ktempdir.h>
#include <ksavefile.h>

#include <qtest_kde.h>

QTEST_KDEMAIN_CORE( KSaveFileTest )

void KSaveFileTest::initTestCase()
{
    tmp = tmpDir.path() + '/';
}

void KSaveFileTest::test_ksavefile()
{
    QString targetFile;

    {
        //This will be the file we eventually write to. Yes, I know you
        //should never remove the temporaryfile and then expect the filename
        //to continue to be unique, but this is a test for crying out loud. :)
        QTemporaryFile file(QDir::tempPath() + QLatin1String("/ksavefiletest_XXXXXX"));
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

#ifndef Q_OS_WIN
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
#ifndef Q_OS_WIN
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

void KSaveFileTest::transactionalWriteNoPermissionsOnDir_data()
{
    QTest::addColumn<bool>("directWriteFallback");

    QTest::newRow("default") << false;
    QTest::newRow("directWriteFallback") << true;
}

void KSaveFileTest::transactionalWriteNoPermissionsOnDir()
{
#ifdef Q_OS_UNIX
    QFETCH(bool, directWriteFallback);
    // Restore permissions so that the QTemporaryDir cleanup can happen
    class PermissionRestorer
    {
        QString m_path;
    public:
        PermissionRestorer(const QString& path)
            : m_path(path)
        {}

        ~PermissionRestorer()
        {
            restore();
        }
        void restore()
        {
            QFile file(m_path);
            file.setPermissions(QFile::Permissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner));
        }
    };


    KTempDir dir;
    QVERIFY(QFile(dir.name()).setPermissions(QFile::ReadOwner | QFile::ExeOwner));
    PermissionRestorer permissionRestorer(dir.name());

    const QString targetFile = dir.name() + QString::fromLatin1("/outfile");
    KSaveFile firstTry(targetFile);
    QVERIFY(!firstTry.open(QIODevice::WriteOnly));
    QCOMPARE((int)firstTry.error(), (int)QFile::PermissionsError); // actually better than QSaveFile (limited because of QTemporaryFileEngine)
    QVERIFY(!firstTry.finalize());

    // Now make an existing writable file
    permissionRestorer.restore();
    QFile f(targetFile);
    QVERIFY(f.open(QIODevice::WriteOnly));
    QCOMPARE(f.write("Hello"), Q_INT64_C(5));
    f.close();

    // Make the directory non-writable again
    QVERIFY(QFile(dir.name()).setPermissions(QFile::ReadOwner | QFile::ExeOwner));

    // And write to it again using KSaveFile; only works if directWriteFallback is enabled
    KSaveFile file(targetFile);
    file.setDirectWriteFallback(directWriteFallback);
    QCOMPARE(file.directWriteFallback(), directWriteFallback);
    if (directWriteFallback) {
        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE((int)file.error(), (int)QFile::NoError);
        QCOMPARE(file.write("World"), Q_INT64_C(5));
        QVERIFY(file.finalize());

        QFile reader(targetFile);
        QVERIFY(reader.open(QIODevice::ReadOnly));
        QCOMPARE(QString::fromLatin1(reader.readAll()), QString::fromLatin1("World"));
        reader.close();

        QVERIFY(file.open(QIODevice::WriteOnly));
        QCOMPARE((int)file.error(), (int)QFile::NoError);
        QCOMPARE(file.write("W"), Q_INT64_C(1));
        QVERIFY(file.finalize());

        QVERIFY(reader.open(QIODevice::ReadOnly));
        QCOMPARE(QString::fromLatin1(reader.readAll()), QString::fromLatin1("W"));
    } else {
        QVERIFY(!file.open(QIODevice::WriteOnly));
        QCOMPARE((int)file.error(), (int)QFile::PermissionsError);
    }
#endif
}



void KSaveFileTest::test_simpleBackupFile()
{
    QTemporaryFile file;
    QVERIFY( file.open() );

    QVERIFY( KBackup::simpleBackupFile(file.fileName()));
    QVERIFY( QFile::exists(file.fileName() + '~'));
    QFile::remove(file.fileName() + '~');

    QVERIFY( KBackup::simpleBackupFile(file.fileName(), tmp) );
    QFileInfo fi ( file.fileName() );
    QVERIFY( QFile::exists(tmp + fi.fileName() + '~') );
    QFile::remove(tmp + fi.fileName() + '~');
}

void KSaveFileTest::test_numberedBackupFile()
{
    // Test absolute pathname file
    {
        QTemporaryFile file;
        QVERIFY( file.open() );

        for ( int i=1; i<15; i++ ) {
            QVERIFY( KBackup::numberedBackupFile(file.fileName()) );
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
        QTemporaryFile file;
        QVERIFY( file.open() );

        QFileInfo fi ( file.fileName() );
        QVERIFY( QDir::setCurrent(fi.absolutePath()) );

        for ( int i=1; i<15; i++ ) {
            QVERIFY( KBackup::numberedBackupFile(fi.fileName()) );
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
        QTemporaryFile file;
        QVERIFY( file.open() );

        QFileInfo fi ( file.fileName() );
        QVERIFY( QDir::setCurrent(fi.absolutePath()) );

        for ( int i=1; i<15; i++ ) {
            QVERIFY( KBackup::numberedBackupFile(fi.fileName(), tmp) );
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
        QTemporaryFile file;
        QVERIFY(file.open());

        QFileInfo fi ( file.fileName() );
        QVERIFY( QDir::setCurrent(fi.absolutePath()) );

        for ( int i=1; i<15; i++ ) {
            QVERIFY( KBackup::numberedBackupFile(fi.fileName(), tmp) );
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
    QString cipath = QStandardPaths::findExecutable("ci");
    if (cipath.isEmpty())
        QSKIP("ci not available");

    {
        QTemporaryFile f;
        QVERIFY(f.open());

        QVERIFY( KBackup::rcsBackupFile( f.fileName() ) );
        QVERIFY( QFile::exists(f.fileName() + ",v" ));
        QVERIFY( KBackup::rcsBackupFile( f.fileName() ) );
        QVERIFY( QFile::exists(f.fileName() + ",v" ));

        filesToRemove << f.fileName() + ",v";
    }

    {
        QTemporaryFile f;
        QVERIFY(f.open());

        QVERIFY( KBackup::rcsBackupFile( f.fileName() ) );
        QVERIFY( QFile::exists(f.fileName() + ",v" ) );

        QTextStream out( &f );
        out << "Testing a change\n";
        out.flush();

        QVERIFY( KBackup::rcsBackupFile( f.fileName() ) );

        out << "Testing another change\n";
        out.flush();

        QVERIFY( KBackup::rcsBackupFile( f.fileName() ) );
        filesToRemove << f.fileName() + ",v";
    }

    {
        QTemporaryFile f;
        QVERIFY(f.open());

        QFileInfo fi ( f.fileName() );
        QVERIFY( QDir::setCurrent(fi.absolutePath()) );

        QVERIFY( KBackup::rcsBackupFile( fi.fileName() ) );
        QVERIFY( QFile::exists(f.fileName() + ",v" ));
        QVERIFY( KBackup::rcsBackupFile( fi.fileName() ) );
        QVERIFY( QFile::exists(f.fileName() + ",v" ));

        filesToRemove << f.fileName() + ",v";
    }

    {
        QTemporaryFile f;
        QVERIFY(f.open());

        QFileInfo fi ( f.fileName() );

        QVERIFY( KBackup::rcsBackupFile( f.fileName(), tmp ) );
        QVERIFY( QFile::exists(tmp + fi.fileName() + ",v" ));
        QVERIFY( KBackup::rcsBackupFile( f.fileName(), tmp ) );
        QVERIFY( QFile::exists(tmp + fi.fileName() + ",v" ));

        filesToRemove << tmp + fi.fileName() + ",v";
    }

    {
        QTemporaryFile f;
        QVERIFY(f.open());

        QFileInfo fi ( f.fileName() );
        QVERIFY( QDir::setCurrent(fi.absolutePath()) );

        QVERIFY( KBackup::rcsBackupFile( fi.fileName(), tmp ) );
        QVERIFY( QFile::exists(tmp + fi.fileName() + ",v" ));
        QVERIFY( KBackup::rcsBackupFile( fi.fileName(), tmp ) );
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
