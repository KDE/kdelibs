/*
  This file is part of the KDE libraries
  Copyright (c) 2006 Allen Winter <winter@kde.org>

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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <QTextStream>

#include <kapplication.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <ktempfile.h>
#include <ksavefile.h>

#include <qtest_kde.h>
#include <ksavefiletest.h>


QTEST_KDEMAIN( KSaveFileTest, NoGUI )


void test( const char *msg, bool result )
{
    fprintf( stderr, "Testing %s .... %s\n", msg, result ? "OK" : "FAILED" );
}

void KSaveFileTest::test_numberedBackupFile()
{
    KTempFile f( "fred", QString(), 0600 );

    test( "backup file", KSaveFile::backupFile( f.name() ) );
    test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
    test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
    test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
    test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
    test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
    test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
    test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
    test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
    test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
    test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
    test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
    test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
    test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
    test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
    test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
    test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
    //test( "numbered backup", KSaveFile::numberedBackupFile( f.name(),5 ) );
    test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
    test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
    test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
    //test( "numbered backup", KSaveFile::numberedBackupFile( f.name(),5 ) );

    f.setAutoDelete( true );
    // TODO also clean up all the numbered backups!
}

void KSaveFileTest::test_rcsBackupFile()
{
    KTempFile f( "fred", QString(), 0600 );

    test( "rcs backup", KSaveFile::rcsBackupFile( f.name() ) );
    test( "rcs backup", KSaveFile::rcsBackupFile( f.name() ) );

    QFile fl( f.name() );
    if ( fl.open( QFile::WriteOnly | QFile::Truncate ) ) {
        QTextStream out( &fl );
        out << "Testing a change\n";
        fl.close();
        test( "rcs backup", KSaveFile::rcsBackupFile( f.name(), QString("/tmp"), "Testmsg" ) );
    }
    test( "rcs backup", KSaveFile::rcsBackupFile( f.name(), QString(), "BROKE IF YOU SEE ME IN /tmp" ) );
    if ( fl.open( QFile::WriteOnly | QFile::Truncate ) ) {
        QTextStream out( &fl );
        out << "Testing another change\n";
        fl.close();
        test( "rcs backup", KSaveFile::rcsBackupFile( f.name(), QString("/tmp"), "Another Testmsg" ) );
    }
    QFile::remove( f.name() + ",v" );
    f.unlink();
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


#include "ksavefiletest.moc"
