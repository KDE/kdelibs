/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>

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

#include "jobtest.h"

#include <config.h>

#include <kurl.h>
#include <kapplication.h>
#include <klargefile.h>
#include <kio/netaccess.h>
#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kprotocolinfo.h>

#include <qfileinfo.h>
#include <qeventloop.h>
#include <qdir.h>
#include <qfileinfo.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <utime.h>

// The code comes partly from kdebase/kioslave/trash/testtrash.cpp

static bool check(const QString& txt, QString a, QString b)
{
    if (a.isEmpty())
        a = QString::null;
    if (b.isEmpty())
        b = QString::null;
    if (a == b) {
        kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "ok" << endl;
    }
    else {
        kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "KO !" << endl;
        exit(1);
    }
    return true;
}

int main(int argc, char *argv[])
{
    KApplication::disableAutoDcopRegistration();
    KCmdLineArgs::init(argc,argv,"jobtest", 0, 0, 0, 0);
    KApplication app;

    JobTest test;
    test.setup();
    test.runAll();
    test.cleanup();
    kdDebug() << "All tests OK." << endl;
    return 0; // success. The exit(1) in check() is what happens in case of failure.
}

QString JobTest::homeTmpDir() const
{
    return QDir::homeDirPath() + "/.kde/jobtest/";
}

QString JobTest::otherTmpDir() const
{
    // This one needs to be on another partition
    return "/tmp/jobtest/";
}

KURL JobTest::systemTmpDir() const
{
    return "system:/home/.kde/jobtest-system/";
}

QString JobTest::realSystemPath() const
{
    return QDir::homeDirPath() + "/.kde/jobtest-system/";
}

void JobTest::setup()
{
    // Start with a clean base dir
    cleanup();
    QDir dir; // TT: why not a static method?
    bool ok = dir.mkdir( homeTmpDir() );
    if ( !ok )
        kdFatal() << "Couldn't create " << homeTmpDir() << endl;
    ok = dir.mkdir( otherTmpDir() );
    if ( !ok )
        kdFatal() << "Couldn't create " << otherTmpDir() << endl;
    ok = dir.mkdir( realSystemPath() );
    if ( !ok )
        kdFatal() << "Couldn't create " << realSystemPath() << endl;
}

void JobTest::runAll()
{
    get();
    copyFileToSamePartition();
    copyDirectoryToSamePartition();
    copyDirectoryToExistingDirectory();
    copyFileToOtherPartition();
    copyDirectoryToOtherPartition();
    listRecursive();
    moveFileToSamePartition();
    moveDirectoryToSamePartition();
    moveFileToOtherPartition();
    moveSymlinkToOtherPartition();
    moveDirectoryToOtherPartition();
    moveFileNoPermissions();
    moveDirectoryNoPermissions();

    copyFileToSystem();
}

void JobTest::cleanup()
{
    KIO::NetAccess::del( homeTmpDir(), 0 );
    KIO::NetAccess::del( otherTmpDir(), 0 );
    KIO::NetAccess::del( systemTmpDir(), 0 );
}

static void setTimeStamp( const QString& path )
{
#ifdef Q_OS_UNIX
    // Put timestamp in the past so that we can check that the
    // copy actually preserves it.
    struct timeval tp;
    gettimeofday( &tp, 0 );
    struct utimbuf utbuf;
    utbuf.actime = tp.tv_sec - 30; // 30 seconds ago
    utbuf.modtime = tp.tv_sec - 60; // 60 second ago
    utime( QFile::encodeName( path ), &utbuf );
    qDebug( "Time changed for %s", path.latin1() );
#endif
}

static void createTestFile( const QString& path )
{
    QFile f( path );
    if ( !f.open( IO_WriteOnly ) )
        kdFatal() << "Can't create " << path << endl;
    f.writeBlock( "Hello world", 11 );
    f.close();
    setTimeStamp( path );
}

static void createTestSymlink( const QString& path )
{
    // Create symlink if it doesn't exist yet
    KDE_struct_stat buf;
    if ( KDE_lstat( QFile::encodeName( path ), &buf ) != 0 ) {
        bool ok = symlink( "/IDontExist", QFile::encodeName( path ) ) == 0; // broken symlink
        if ( !ok )
            kdFatal() << "couldn't create symlink: " << strerror( errno ) << endl;
    }
}

static void createTestDirectory( const QString& path )
{
    QDir dir;
    bool ok = dir.mkdir( path );
    if ( !ok && !dir.exists() )
        kdFatal() << "couldn't create " << path << endl;
    createTestFile( path + "/testfile" );
    createTestSymlink( path + "/testlink" );
    setTimeStamp( path );
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
    kapp->eventLoop()->enterLoop();
    assert( m_result == 0 ); // no error
    assert( m_data.size() == 11 );
    assert( QCString( m_data ) == "Hello world" );
}

void JobTest::slotGetResult( KIO::Job* job )
{
    m_result = job->error();
    m_data = static_cast<KIO::StoredTransferJob *>(job)->data();
    kapp->eventLoop()->exitLoop();
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
    assert( ok );
    assert( QFile::exists( dest ) );
    assert( QFile::exists( src ) ); // still there

    {
        // check that the timestamp is the same (#24443)
        // Note: this only works because of copy() in kio_file.
        // The datapump solution ignores mtime, the app has to call FileCopyJob::setModificationTime()
        QFileInfo srcInfo( src );
        QFileInfo destInfo( dest );
        assert( srcInfo.lastModified() == destInfo.lastModified() );
    }

    // cleanup and retry with KIO::copy()
    QFile::remove( dest );
    ok = KIO::NetAccess::dircopy( u, d, 0 );
    assert( ok );
    assert( QFile::exists( dest ) );
    assert( QFile::exists( src ) ); // still there
    {
        // check that the timestamp is the same (#24443)
        QFileInfo srcInfo( src );
        QFileInfo destInfo( dest );
        assert( srcInfo.lastModified() == destInfo.lastModified() );
    }
}

void JobTest::copyLocalDirectory( const QString& src, const QString& _dest, int flags )
{
    assert( QFileInfo( src ).isDir() );
    assert( QFileInfo( src + "/testfile" ).isFile() );
    KURL u;
    u.setPath( src );
    QString dest( _dest );
    KURL d;
    d.setPath( dest );
    if ( flags & AlreadyExists )
        assert( QFile::exists( dest ) );
    else
        assert( !QFile::exists( dest ) );

    bool ok = KIO::NetAccess::dircopy( u, d, 0 );
    assert( ok );

    if ( flags & AlreadyExists ) {
        dest += "/" + u.fileName();
        //kdDebug() << "Expecting dest=" << dest << endl;
    }

    assert( QFile::exists( dest ) );
    assert( QFileInfo( dest ).isDir() );
    assert( QFileInfo( dest + "/testfile" ).isFile() );
    assert( QFile::exists( src ) ); // still there
    {
        // check that the timestamp is the same (#24443)
        QFileInfo srcInfo( src );
        QFileInfo destInfo( dest );
        assert( srcInfo.lastModified() == destInfo.lastModified() );
    }
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

void JobTest::copyDirectoryToExistingDirectory()
{
    kdDebug() << k_funcinfo << endl;
    // just the same as copyDirectoryToSamePartition, but it means that
    // this time dest exists.
    const QString src = homeTmpDir() + "dirFromHome";
    const QString dest = homeTmpDir() + "dirFromHome_copied";
    createTestDirectory( src );
    copyLocalDirectory( src, dest, AlreadyExists );
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
    const QString dest = otherTmpDir() + "dirFromHome_copied";
    // src is already created by copyDirectoryToSamePartition()
    // so this is just in case someone calls this method only
    if ( !QFile::exists( src ) )
        createTestDirectory( src );
    copyLocalDirectory( src, dest );
}

void JobTest::moveLocalFile( const QString& src, const QString& dest )
{
    assert( QFile::exists( src ) );
    KURL u;
    u.setPath( src );
    KURL d;
    d.setPath( dest );

    // move the file with file_move
    bool ok = KIO::NetAccess::file_move( u, d );
    assert( ok );
    assert( QFile::exists( dest ) );
    assert( !QFile::exists( src ) ); // not there anymore

    // move it back with KIO::move()
    ok = KIO::NetAccess::move( d, u, 0 );
    assert( ok );
    assert( !QFile::exists( dest ) );
    assert( QFile::exists( src ) ); // it's back
}

static void moveLocalSymlink( const QString& src, const QString& dest )
{
    KDE_struct_stat buf;
    assert ( KDE_lstat( QFile::encodeName( src ), &buf ) == 0 );
    KURL u;
    u.setPath( src );
    KURL d;
    d.setPath( dest );

    // move the symlink with move, NOT with file_move
    bool ok = KIO::NetAccess::move( u, d );
    if ( !ok )
        kdWarning() << KIO::NetAccess::lastError() << endl;
    assert( ok );
    assert ( KDE_lstat( QFile::encodeName( dest ), &buf ) == 0 );
    assert( !QFile::exists( src ) ); // not there anymore

    // move it back with KIO::move()
    ok = KIO::NetAccess::move( d, u, 0 );
    assert( ok );
    assert ( KDE_lstat( QFile::encodeName( dest ), &buf ) != 0 ); // doesn't exist anymore
    assert ( KDE_lstat( QFile::encodeName( src ), &buf ) == 0 ); // it's back
}

void JobTest::moveLocalDirectory( const QString& src, const QString& dest )
{
    assert( QFile::exists( src ) );
    assert( QFileInfo( src ).isDir() );
    assert( QFileInfo( src + "/testfile" ).isFile() );
    assert( QFileInfo( src + "/testlink" ).isSymLink() );
    KURL u;
    u.setPath( src );
    KURL d;
    d.setPath( dest );

    bool ok = KIO::NetAccess::move( u, d, 0 );
    assert( ok );
    assert( QFile::exists( dest ) );
    assert( QFileInfo( dest ).isDir() );
    assert( QFileInfo( dest + "/testfile" ).isFile() );
    assert( !QFile::exists( src ) ); // not there anymore

    assert( QFileInfo( dest + "/testlink" ).isSymLink() );
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
    assert( QFile::exists( src ) );
    assert( QFileInfo( src ).isFile() );
    KURL u;
    u.setPath( src );
    KURL d;
    d.setPath( dest );

    KIO::CopyJob* job = KIO::move( u, d, 0 );
    job->setInteractive( false ); // no skip dialog, thanks
    QMap<QString, QString> metaData;
    bool ok = KIO::NetAccess::synchronousRun( job, 0, 0, 0, &metaData );
    assert( !ok );
    assert( KIO::NetAccess::lastError() == KIO::ERR_ACCESS_DENIED );
    // OK this is fishy. Just like mv(1), KIO's behavior depends on whether
    // a direct rename(2) was used, or a full copy+del. In the first case
    // there is no destination file created, but in the second case the
    // destination file remains.
    // In fact we assume /home is a separate partition, in this test, so:
    assert( QFile::exists( dest ) );
    assert( QFile::exists( src ) );
}

void JobTest::moveDirectoryNoPermissions()
{
    kdDebug() << k_funcinfo << endl;
    const QString src = "/etc/init.d";
    const QString dest = homeTmpDir() + "init.d";
    assert( QFile::exists( src ) );
    assert( QFileInfo( src ).isDir() );
    KURL u;
    u.setPath( src );
    KURL d;
    d.setPath( dest );

    KIO::CopyJob* job = KIO::move( u, d, 0 );
    job->setInteractive( false ); // no skip dialog, thanks
    QMap<QString, QString> metaData;
    bool ok = KIO::NetAccess::synchronousRun( job, 0, 0, 0, &metaData );
    assert( !ok );
    assert( KIO::NetAccess::lastError() == KIO::ERR_ACCESS_DENIED );
    assert( QFile::exists( dest ) ); // see moveFileNoPermissions
    assert( QFile::exists( src ) );
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
    assert( ok );
    m_names.sort();
    check( "listRecursive", m_names.join( "," ), ".,..,"
            "dirFromHome,dirFromHome/testfile,dirFromHome/testlink,dirFromHome_copied,"
            "dirFromHome_copied/dirFromHome,dirFromHome_copied/dirFromHome/testfile,dirFromHome_copied/dirFromHome/testlink,"
            "dirFromHome_copied/testfile,dirFromHome_copied/testlink,"
            "fileFromHome,fileFromHome_copied" );
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

void JobTest::copyFileToSystem()
{
    if ( !KProtocolInfo::isKnownProtocol( QString::fromLatin1( "system" ) ) ) {
        kdDebug() << k_funcinfo << "no kio_system, skipping test" << endl;
        return;
    }

    // First test with support for UDS_LOCAL_PATH
    copyFileToSystem( true );

    QString dest = realSystemPath() + "fileFromHome_copied";
    QFile::remove( dest );

    // Then disable support for UDS_LOCAL_PATH, i.e. test what would
    // happen for ftp, smb, http etc.
    copyFileToSystem( false );
}

void JobTest::copyFileToSystem( bool resolve_local_urls )
{
    kdDebug() << k_funcinfo << resolve_local_urls << endl;
    extern KIO_EXPORT bool kio_resolve_local_urls;
    kio_resolve_local_urls = resolve_local_urls;

    const QString src = homeTmpDir() + "fileFromHome";
    createTestFile( src );
    KURL u;
    u.setPath( src );
    KURL d = systemTmpDir();
    d.addPath( "fileFromHome_copied" );

    kdDebug() << "copying " << u << " to " << d << endl;

    // copy the file with file_copy
    KIO::FileCopyJob* job = KIO::file_copy( u, d );
    connect( job, SIGNAL(mimetype(KIO::Job*,const QString&)),
             this, SLOT(slotMimetype(KIO::Job*,const QString&)) );
    bool ok = KIO::NetAccess::synchronousRun( job, 0 );
    assert( ok );

    QString dest = realSystemPath() + "fileFromHome_copied";

    assert( QFile::exists( dest ) );
    assert( QFile::exists( src ) ); // still there

    {
        // do NOT check that the timestamp is the same.
        // It can't work with file_copy when it uses the datapump,
        // unless we use setModificationTime in the app code.
    }

    // Check mimetype
    kdDebug() << m_mimetype << endl;
    // There's no mimemagic determination in kio_file in kde3. Fixing this for kde4...
    assert( m_mimetype == "application/octet-stream" );
    //assert( m_mimetype == "text/plain" );

    // cleanup and retry with KIO::copy()
    QFile::remove( dest );
    ok = KIO::NetAccess::dircopy( u, d, 0 );
    assert( ok );
    assert( QFile::exists( dest ) );
    assert( QFile::exists( src ) ); // still there
    {
        // check that the timestamp is the same (#79937)
        QFileInfo srcInfo( src );
        QFileInfo destInfo( dest );
        assert( srcInfo.lastModified() == destInfo.lastModified() );
    }

    // restore normal behavior
    kio_resolve_local_urls = true;
}

void JobTest::slotMimetype(KIO::Job* job, const QString& type)
{
    assert( job );
    m_mimetype = type;
}

#include "jobtest.moc"
