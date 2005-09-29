/* This file is part of the KDE project
   Copyright (C) 2004, 2005 David Faure <faure@kde.org>

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

#include "QtTest/qttest_kde.h"

#include "jobtest.h"

#include <config.h>

#include <kurl.h>
#include <kapplication.h>
#include <kde_file.h>
#include <kio/netaccess.h>
#include <kdebug.h>
#include <kcmdlineargs.h>

#include <qfileinfo.h>
#include <qeventloop.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qhash.h>
#include <qvariant.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

QTTEST_KDEMAIN( JobTest, GUI )

// The code comes partly from kdebase/kioslave/trash/testtrash.cpp

QString JobTest::homeTmpDir() const
{
    return QDir::homePath() + "/.kde/jobtest/";
}

QString JobTest::otherTmpDir() const
{
    // This one needs to be on another partition
    return "/tmp/jobtest/";
}

void JobTest::initTestCase()
{
    qDebug( "initTestCase" );
    // Start with a clean base dir
    cleanupTestCase();
    QDir dir; // TT: why not a static method?
    bool ok = dir.mkdir( homeTmpDir() );
    if ( !ok )
        kdFatal() << "Couldn't create " << homeTmpDir() << endl;
    ok = dir.mkdir( otherTmpDir() );
    if ( !ok )
        kdFatal() << "Couldn't create " << otherTmpDir() << endl;
}

void JobTest::cleanupTestCase()
{
    KIO::NetAccess::del( homeTmpDir(), 0 );
    KIO::NetAccess::del( otherTmpDir(), 0 );
}

static void createTestFile( const QString& path )
{
    QFile f( path );
    if ( !f.open( QIODevice::WriteOnly ) )
        kdFatal() << "Can't create " << path << endl;
    f.writeBlock( "Hello world", 11 );
    f.close();
}

static void createTestSymlink( const QString& path )
{
    // Create symlink if it doesn't exist yet
    KDE_struct_stat buf;
    if ( KDE_lstat( QFile::encodeName( path ), &buf ) != 0 ) {
        bool ok = symlink( "/IDontExist", QFile::encodeName( path ) ) == 0; // broken symlink
        if ( !ok )
            kdFatal() << "couldn't create symlink: " << strerror( errno ) << endl;
        VERIFY( KDE_lstat( QFile::encodeName( path ), &buf ) == 0 );
        VERIFY( S_ISLNK( buf.st_mode ) );
    } else {
        VERIFY( S_ISLNK( buf.st_mode ) );
    }
    qDebug( "symlink %s created", qPrintable( path ) );
    VERIFY( QFileInfo( path ).isSymLink() );
}

static void createTestDirectory( const QString& path )
{
    QDir dir;
    bool ok = dir.mkdir( path );
    if ( !ok && !dir.exists() )
        kdFatal() << "couldn't create " << path << endl;
    createTestFile( path + "/testfile" );
    createTestSymlink( path + "/testlink" );
    VERIFY( QFileInfo( path + "/testlink" ).isSymLink() );
}

void JobTest::enterLoop()
{
    QEventLoop eventLoop;
    connect(this, SIGNAL(exitLoop()),
            &eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
}

void JobTest::get()
{
    kdDebug() << k_funcinfo << endl;
    const QString filePath = homeTmpDir() + "fileFromHome";
    createTestFile( filePath );
    KURL u; u.setPath( filePath );
    m_result = -1;
    KIO::StoredTransferJob* job = KIO::storedGet( u );
    connect( job, SIGNAL( result( KIO::Job* ) ),
            this, SLOT( slotGetResult( KIO::Job* ) ) );
    enterLoop();
    VERIFY( m_result == 0 ); // no error
    VERIFY( m_data.size() == 11 );
    VERIFY( QByteArray( m_data ) == "Hello world" );
}

void JobTest::slotGetResult( KIO::Job* job )
{
    m_result = job->error();
    m_data = static_cast<KIO::StoredTransferJob *>(job)->data();
    emit exitLoop();
}

////

void JobTest::copyLocalFile( const QString& src, const QString& dest )
{
    KURL u;
    u.setPath( src );
    KURL d;
    d.setPath( dest );

    // copy the file with file_copy
    bool ok = KIO::NetAccess::file_copy( u, d );
    VERIFY( ok );
    VERIFY( QFile::exists( dest ) );
    VERIFY( QFile::exists( src ) ); // still there

    // cleanup and retry with KIO::copy()
    QFile::remove( dest );
    ok = KIO::NetAccess::dircopy( u, d, 0 );
    VERIFY( ok );
    VERIFY( QFile::exists( dest ) );
    VERIFY( QFile::exists( src ) ); // still there
}

void JobTest::copyLocalDirectory( const QString& src, const QString& dest )
{
    VERIFY( QFileInfo( src ).isDir() );
    VERIFY( QFileInfo( src + "/testfile" ).isFile() );
    KURL u;
    u.setPath( src );
    KURL d;
    d.setPath( dest );

    bool ok = KIO::NetAccess::dircopy( u, d, 0 );
    VERIFY( ok );
    VERIFY( QFile::exists( dest ) );
    VERIFY( QFileInfo( dest ).isDir() );
    VERIFY( QFileInfo( dest + "/testfile" ).isFile() );
    VERIFY( QFile::exists( src ) ); // still there
}

void JobTest::copyFileToSamePartition()
{
    kdDebug() << k_funcinfo << endl;
    const QString filePath = homeTmpDir() + "fileFromHome";
    const QString dest = homeTmpDir() + "fileFromHome_copied";
    createTestFile( filePath );
    copyLocalFile( filePath, dest );
}

void JobTest::copyDirectoryToSamePartition()
{
    kdDebug() << k_funcinfo << endl;
    const QString src = homeTmpDir() + "dirFromHome";
    const QString dest = homeTmpDir() + "dirFromHome_copied";
    createTestDirectory( src );
    copyLocalDirectory( src, dest );
}

void JobTest::copyFileToOtherPartition()
{
    kdDebug() << k_funcinfo << endl;
    const QString filePath = homeTmpDir() + "fileFromHome";
    const QString dest = otherTmpDir() + "fileFromHome_copied";
    createTestFile( filePath );
    copyLocalFile( filePath, dest );
}

void JobTest::copyDirectoryToOtherPartition()
{
    kdDebug() << k_funcinfo << endl;
    const QString src = homeTmpDir() + "dirFromHome";
    const QString dest = homeTmpDir() + "dirFromHome_copied";
    // src is already created by copyDirectoryToSamePartition()
    copyLocalDirectory( src, dest );
}

void JobTest::moveLocalFile( const QString& src, const QString& dest )
{
    VERIFY( QFile::exists( src ) );
    KURL u;
    u.setPath( src );
    KURL d;
    d.setPath( dest );

    // move the file with file_move
    bool ok = KIO::NetAccess::file_move( u, d );
    VERIFY( ok );
    VERIFY( QFile::exists( dest ) );
    VERIFY( !QFile::exists( src ) ); // not there anymore

    // move it back with KIO::move()
    ok = KIO::NetAccess::move( d, u, 0 );
    VERIFY( ok );
    VERIFY( !QFile::exists( dest ) );
    VERIFY( QFile::exists( src ) ); // it's back
}

static void moveLocalSymlink( const QString& src, const QString& dest )
{
    KDE_struct_stat buf;
    VERIFY ( KDE_lstat( QFile::encodeName( src ), &buf ) == 0 );
    KURL u;
    u.setPath( src );
    KURL d;
    d.setPath( dest );

    // move the symlink with move, NOT with file_move
    bool ok = KIO::NetAccess::move( u, d );
    if ( !ok )
        kdWarning() << KIO::NetAccess::lastError() << endl;
    VERIFY( ok );
    VERIFY ( KDE_lstat( QFile::encodeName( dest ), &buf ) == 0 );
    VERIFY( !QFile::exists( src ) ); // not there anymore

    // move it back with KIO::move()
    ok = KIO::NetAccess::move( d, u, 0 );
    VERIFY( ok );
    VERIFY ( KDE_lstat( QFile::encodeName( dest ), &buf ) != 0 ); // doesn't exist anymore
    VERIFY ( KDE_lstat( QFile::encodeName( src ), &buf ) == 0 ); // it's back
}

void JobTest::moveLocalDirectory( const QString& src, const QString& dest )
{
    kdDebug() << k_funcinfo << src << " " << dest << endl;
    VERIFY( QFile::exists( src ) );
    VERIFY( QFileInfo( src ).isDir() );
    VERIFY( QFileInfo( src + "/testfile" ).isFile() );
    VERIFY( QFileInfo( src + "/testlink" ).isSymLink() );
    KURL u;
    u.setPath( src );
    KURL d;
    d.setPath( dest );

    bool ok = KIO::NetAccess::move( u, d, 0 );
    VERIFY( ok );
    VERIFY( QFile::exists( dest ) );
    VERIFY( QFileInfo( dest ).isDir() );
    VERIFY( QFileInfo( dest + "/testfile" ).isFile() );
    VERIFY( !QFile::exists( src ) ); // not there anymore

    VERIFY( QFileInfo( dest + "/testlink" ).isSymLink() );
}

void JobTest::moveFileToSamePartition()
{
    kdDebug() << k_funcinfo << endl;
    const QString filePath = homeTmpDir() + "fileFromHome";
    const QString dest = homeTmpDir() + "fileFromHome_moved";
    createTestFile( filePath );
    moveLocalFile( filePath, dest );
}

void JobTest::moveDirectoryToSamePartition()
{
    kdDebug() << k_funcinfo << endl;
    const QString src = homeTmpDir() + "dirFromHome";
    const QString dest = homeTmpDir() + "dirFromHome_moved";
    createTestDirectory( src );
    moveLocalDirectory( src, dest );
}

void JobTest::moveFileToOtherPartition()
{
    kdDebug() << k_funcinfo << endl;
    const QString filePath = homeTmpDir() + "fileFromHome";
    const QString dest = otherTmpDir() + "fileFromHome_moved";
    createTestFile( filePath );
    moveLocalFile( filePath, dest );
}

void JobTest::moveSymlinkToOtherPartition()
{
    kdDebug() << k_funcinfo << endl;
    const QString filePath = homeTmpDir() + "testlink";
    const QString dest = otherTmpDir() + "testlink_moved";
    createTestSymlink( filePath );
    moveLocalSymlink( filePath, dest );
}

void JobTest::moveDirectoryToOtherPartition()
{
    kdDebug() << k_funcinfo << endl;
    const QString src = homeTmpDir() + "dirFromHome";
    const QString dest = otherTmpDir() + "dirFromHome_moved";
    createTestDirectory( src );
    moveLocalDirectory( src, dest );
}

void JobTest::moveFileNoPermissions()
{
    kdDebug() << k_funcinfo << endl;
    const QString src = "/etc/passwd";
    const QString dest = homeTmpDir() + "passwd";
    VERIFY( QFile::exists( src ) );
    VERIFY( QFileInfo( src ).isFile() );
    KURL u;
    u.setPath( src );
    KURL d;
    d.setPath( dest );

    KIO::CopyJob* job = KIO::move( u, d, 0 );
    job->setInteractive( false ); // no skip dialog, thanks
    QMap<QString, QString> metaData;
    bool ok = KIO::NetAccess::synchronousRun( job, 0, 0, 0, &metaData );
    VERIFY( !ok );
    VERIFY( KIO::NetAccess::lastError() == KIO::ERR_ACCESS_DENIED );
    // OK this is fishy. Just like mv(1), KIO's behavior depends on whether
    // a direct rename(2) was used, or a full copy+del. In the first case
    // there is no destination file created, but in the second case the
    // destination file remains.
    // In fact we assume /home is a separate partition, in this test, so:
    VERIFY( QFile::exists( dest ) );
    VERIFY( QFile::exists( src ) );
}

void JobTest::moveDirectoryNoPermissions()
{
    kdDebug() << k_funcinfo << endl;
    const QString src = "/etc";
    const QString dest = homeTmpDir() + "etc";
    VERIFY( QFile::exists( src ) );
    VERIFY( QFileInfo( src ).isDir() );
    KURL u;
    u.setPath( src );
    KURL d;
    d.setPath( dest );

    KIO::CopyJob* job = KIO::move( u, d, 0 );
    job->setInteractive( false ); // no skip dialog, thanks
    QMap<QString, QString> metaData;
    bool ok = KIO::NetAccess::synchronousRun( job, 0, 0, 0, &metaData );
    VERIFY( !ok );
    VERIFY( KIO::NetAccess::lastError() == KIO::ERR_ACCESS_DENIED );
    VERIFY( QFile::exists( dest ) ); // see moveFileNoPermissions
    VERIFY( QFile::exists( src ) );
}

void JobTest::listRecursive()
{
    const QString src = homeTmpDir();
    KURL u;
    u.setPath( src );
    KIO::ListJob* job = KIO::listRecursive( u );
    connect( job, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList& ) ),
             SLOT( slotEntries( KIO::Job*, const KIO::UDSEntryList& ) ) );
    bool ok = KIO::NetAccess::synchronousRun( job, 0 );
    VERIFY( ok );
    m_names.sort();
    COMPARE( m_names.join( "," ).toLatin1(), QByteArray( ".,..,"
            "dirFromHome,dirFromHome/testfile,dirFromHome/testlink,dirFromHome_copied,"
            "dirFromHome_copied/dirFromHome,dirFromHome_copied/dirFromHome/testfile,dirFromHome_copied/dirFromHome/testlink,"
            "dirFromHome_copied/testfile,dirFromHome_copied/testlink,"
            "fileFromHome,fileFromHome_copied" ) );
}

void JobTest::slotEntries( KIO::Job*, const KIO::UDSEntryList& lst )
{
    for( KIO::UDSEntryList::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
        KIO::UDSEntry::ConstIterator it2 = (*it).begin();
        QString displayName;
        KURL url;
        for( ; it2 != (*it).end(); it2++ ) {
            switch ((*it2).m_uds) {
            case KIO::UDS_NAME:
                displayName = (*it2).m_str;
                break;
            case KIO::UDS_URL:
                url = (*it2).m_str;
                break;
            }
        }
        m_names.append( displayName );
    }
}

static void fillOldUDSEntry( KIO::UDSEntry& entry, const time_t& now_time_t, const QString& nameStr )
{
    KIO::UDSAtom atom;
    atom.m_uds = KIO::UDS_NAME;
    atom.m_str = nameStr;
    entry.append( atom );
    atom.m_uds = KIO::UDS_SIZE;
    atom.m_long = 123456;
    entry.append( atom );
    atom.m_uds = KIO::UDS_MODIFICATION_TIME;
    atom.m_long = now_time_t;
    entry.append( atom );
    atom.m_uds = KIO::UDS_ACCESS_TIME;
    atom.m_long = now_time_t;
    entry.append( atom );
    atom.m_uds = KIO::UDS_FILE_TYPE;
    atom.m_long = S_IFREG;
    entry.append( atom );
    atom.m_uds = KIO::UDS_ACCESS;
    atom.m_long = 0644;
    entry.append( atom );
    atom.m_uds = KIO::UDS_USER;
    atom.m_str = nameStr;
    entry.append( atom );
    atom.m_uds = KIO::UDS_GROUP;
    atom.m_str = nameStr;
    entry.append( atom );
}

// QHash or QMap? doesn't seem to make much difference.
typedef QHash<uint, QVariant> UDSEntry4;

static void fillNewUDSEntry( UDSEntry4& entry, const QDateTime& now, const QString& nameStr )
{
    entry.insert( KIO::UDS_NAME, nameStr );
    entry.insert( KIO::UDS_SIZE, 123456 );
    entry.insert( KIO::UDS_MODIFICATION_TIME, now );
    entry.insert( KIO::UDS_ACCESS_TIME, now );
    entry.insert( KIO::UDS_FILE_TYPE, S_IFREG );
    entry.insert( KIO::UDS_ACCESS, 0644 );
    entry.insert( KIO::UDS_USER, nameStr );
    entry.insert( KIO::UDS_GROUP, nameStr );
}

void JobTest::newApiPerformance()
{
    const QDateTime now = QDateTime::currentDateTime();
    const time_t now_time_t = now.toTime_t();
    const int iterations = 3000000;
    const QString nameStr = QString::fromLatin1( "name" );

    /*
      This is to compare the old list-of-lists API vs a QMap/QHash-based API
      in terms of performance.

      The number of atoms and their type map to what kio_file would put in
      for any normal file.

      The lookups are done for two atoms that are present, and for one that is not.

      The results are .... not too good right now.

    */

    /// Old API
    {
        qDebug( "Timing old api..." );

        // Slave code
        time_t start = time(0);
        for (int i = 0; i < iterations; ++i) {
            KIO::UDSEntry entry;
            fillOldUDSEntry( entry, now_time_t, nameStr );
        }
        qDebug("Old API: slave code: %ld", time(0) - start);

        KIO::UDSEntry entry;
        fillOldUDSEntry( entry, now_time_t, nameStr );
        COMPARE( entry.count(), 8 );

        start = time(0);

        // App code

        QString displayName;
        KIO::filesize_t size;
        KURL url;

        for (int i = 0; i < iterations; ++i) {
            KIO::UDSEntry::ConstIterator it2 = entry.begin();
            for( ; it2 != entry.end(); it2++ ) {
                switch ((*it2).m_uds) {
                case KIO::UDS_NAME:
                    displayName = (*it2).m_str;
                    break;
                case KIO::UDS_URL:
                    url = (*it2).m_str;
                    break;
                case KIO::UDS_SIZE:
                    size = (*it2).m_long;
                    break;
                }
            }
        }

        qDebug("Old API: app code: %ld", time(0) - start);

        COMPARE( size, 123456ULL );
        COMPARE( displayName, QString::fromLatin1( "name" ) );
        VERIFY( url.isEmpty() );
    }

    ///////// TEST CODE FOR FUTURE KIO API


    ////

    {
        qDebug( "Timing new api..." );

        // Slave code
        time_t start = time(0);
        for (int i = 0; i < iterations; ++i) {
            UDSEntry4 entry;
            fillNewUDSEntry( entry, now, nameStr );
        }

        qDebug("New API: slave code: %ld", time(0) - start);

        UDSEntry4 entry;
        fillNewUDSEntry( entry, now, nameStr );
        COMPARE( entry.count(), 8 );

        start = time(0);

        // App code

        // Normally the code would look like this, but let's change it to time it like the old api
        /*
        QString displayName = entry.value( KIO::UDS_NAME ).toString();
        KURL url = entry.value( KIO::UDS_URL ).toString();
        KIO::filesize_t size = entry.value( KIO::UDS_SIZE ).toULongLong();
        */

        QString displayName;
        KIO::filesize_t size;
        KURL url;

        for (int i = 0; i < iterations; ++i) {
            displayName = entry.value( KIO::UDS_NAME ).toString();
            url = entry.value( KIO::UDS_URL ).toString();
            size = entry.value( KIO::UDS_SIZE ).toULongLong();
        }

        qDebug("New API: app code: %ld", time(0) - start);

        COMPARE( size, 123456ULL );
        COMPARE( displayName, QString::fromLatin1( "name" ) );
        VERIFY( url.isEmpty() );

        /*
        typedef QList<UDSEntry4> UDSEntryList4;
        UDSEntryList4 lst4;
        for( UDSEntryList4::ConstIterator it = lst4.begin(); it != lst4.end(); ++it ) {
            QString displayName = entry.value( KIO::UDS_NAME ).toString();
            KURL url = entry.value( KIO::UDS_URL ).toString();
        }
        */
    }

}

#include "jobtest.moc"
