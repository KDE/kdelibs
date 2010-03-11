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

#include "qtest_kde.h"

#include "jobtest.h"

#include <config.h>

#include <kurl.h>
#include <kio/netaccess.h>
#include <kio/previewjob.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include <QtCore/QFileInfo>
#include <QtCore/QEventLoop>
#include <QtCore/QDir>
#include <QtCore/QHash>
#include <QtCore/QVariant>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <kprotocolinfo.h>
#include <kio/scheduler.h>
#include <kio/directorysizejob.h>
#include <kio/copyjob.h>
#include <kio/deletejob.h>
#include "kiotesthelper.h" // createTestFile etc.

QTEST_KDEMAIN( JobTest, NoGUI )

// The code comes partly from kdebase/kioslave/trash/testtrash.cpp

static QString otherTmpDir()
{
#ifdef Q_WS_WIN
    return QDir::tempPath() + "/jobtest/";
#else
    // This one needs to be on another partition
    return "/tmp/jobtest/";
#endif
}

#if 0
static KUrl systemTmpDir()
{
#ifdef Q_WS_WIN
    return KUrl( "system:" + QDir::homePath() + "/.kde-unit-test/jobtest-system/" );
#else
    return KUrl( "system:/home/.kde-unit-test/jobtest-system/" );
#endif
}

static QString realSystemPath()
{
    return QFile::decodeName( getenv( "KDEHOME" ) ) + "/jobtest-system/";
}
#endif

Q_DECLARE_METATYPE(KIO::Job*)

void JobTest::initTestCase()
{
    s_referenceTimeStamp = QDateTime::currentDateTime().addSecs( -30 ); // 30 seconds ago

    // Start with a clean base dir
    cleanupTestCase();
    homeTmpDir(); // create it
    if ( !QFile::exists( otherTmpDir() ) ) {
        bool ok = QDir().mkdir( otherTmpDir() );
        if ( !ok )
            kFatal() << "Couldn't create " << otherTmpDir();
    }
#if 0
    if ( KProtocolInfo::isKnownProtocol( "system" ) ) {
        if ( !QFile::exists( realSystemPath() ) ) {
            bool ok = dir.mkdir( realSystemPath() );
            if ( !ok )
                kFatal() << "Couldn't create " << realSystemPath();
        }
    }
#endif

    qRegisterMetaType<KJob*>("KJob*");
    qRegisterMetaType<KIO::Job*>("KIO::Job*");
    qRegisterMetaType<KUrl>("KUrl");
    qRegisterMetaType<time_t>("time_t");
}

static void delDir(const QString& pathOrUrl) {
    KIO::Job* job = KIO::del(KUrl(pathOrUrl), KIO::HideProgressInfo);
    job->setUiDelegate(0);
    KIO::NetAccess::synchronousRun(job, 0);
}


void JobTest::cleanupTestCase()
{
    delDir( homeTmpDir() );
    delDir( otherTmpDir() );
#if 0
    if ( KProtocolInfo::isKnownProtocol( "system" ) ) {
        delDir(systemTmpDir());
    }
#endif
}

void JobTest::enterLoop()
{
    QEventLoop eventLoop;
    connect(this, SIGNAL(exitLoop()),
            &eventLoop, SLOT(quit()));
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
}

void JobTest::storedGet()
{
    kDebug() ;
    const QString filePath = homeTmpDir() + "fileFromHome";
    createTestFile( filePath );
    KUrl u( filePath );
    m_result = -1;

    KIO::StoredTransferJob* job = KIO::storedGet( u, KIO::NoReload, KIO::HideProgressInfo );
    QSignalSpy spyPercent(job, SIGNAL(percent(KJob*, unsigned long)));
    QVERIFY(spyPercent.isValid());
    job->setUiDelegate( 0 );
    connect( job, SIGNAL( result( KJob* ) ),
            this, SLOT( slotGetResult( KJob* ) ) );
    enterLoop();
    QCOMPARE( m_result, 0 ); // no error
    QCOMPARE( m_data, QByteArray("Hello\0world", 11) );
    QCOMPARE( m_data.size(), 11 );
    QVERIFY(!spyPercent.isEmpty());
}

void JobTest::slotGetResult( KJob* job )
{
    m_result = job->error();
    m_data = static_cast<KIO::StoredTransferJob *>(job)->data();
    emit exitLoop();
}

void JobTest::put()
{
    const QString filePath = homeTmpDir() + "fileFromHome";
    KUrl u(filePath);
    KIO::TransferJob* job = KIO::put( u, 0600, KIO::Overwrite | KIO::HideProgressInfo );
    QDateTime mtime = QDateTime::currentDateTime().addSecs( -30 ); // 30 seconds ago
    mtime.setTime_t(mtime.toTime_t()); // hack for losing the milliseconds
    job->setModificationTime(mtime);
    job->setUiDelegate( 0 );
    connect( job, SIGNAL( result(KJob*) ),
            this, SLOT( slotResult(KJob*) ) );
    connect( job, SIGNAL(dataReq(KIO::Job*, QByteArray&)),
             this, SLOT(slotDataReq(KIO::Job*, QByteArray&)) );
    m_result = -1;
    m_dataReqCount = 0;
    enterLoop();
    QVERIFY( m_result == 0 ); // no error

    QFileInfo fileInfo(filePath);
    QVERIFY(fileInfo.exists());
    QCOMPARE(fileInfo.size(), 30LL); // "This is a test for KIO::put()\n"
    QCOMPARE((int)fileInfo.permissions(), (int)(QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::WriteUser ));
    QCOMPARE(fileInfo.lastModified(), mtime);
}

void JobTest::slotDataReq( KIO::Job*, QByteArray& data )
{
    // Really not the way you'd write a slotDataReq usually :)
    switch(m_dataReqCount++) {
    case 0:
        data = "This is a test for ";
        break;
    case 1:
        data = "KIO::put()\n";
        break;
    case 2:
        data = QByteArray();
        break;
    }
}

void JobTest::slotResult( KJob* job )
{
    m_result = job->error();
    emit exitLoop();
}

void JobTest::storedPut()
{
    const QString filePath = homeTmpDir() + "fileFromHome";
    KUrl u(filePath);
    QByteArray putData = "This is the put data";
    KIO::TransferJob* job = KIO::storedPut( putData, u, 0600, KIO::Overwrite | KIO::HideProgressInfo );
    QSignalSpy spyPercent(job, SIGNAL(percent(KJob*, unsigned long)));
    QVERIFY(spyPercent.isValid());
    QDateTime mtime = QDateTime::currentDateTime().addSecs( -30 ); // 30 seconds ago
    mtime.setTime_t(mtime.toTime_t()); // hack for losing the milliseconds
    job->setModificationTime(mtime);
    job->setUiDelegate( 0 );
    QVERIFY(job->exec());
    QFileInfo fileInfo(filePath);
    QVERIFY(fileInfo.exists());
    QCOMPARE(fileInfo.size(), (long long)putData.size());
    QCOMPARE((int)fileInfo.permissions(), (int)(QFile::ReadOwner | QFile::WriteOwner | QFile::ReadUser | QFile::WriteUser ));
    QCOMPARE(fileInfo.lastModified(), mtime);
    QVERIFY(!spyPercent.isEmpty());
}

////

void JobTest::copyLocalFile( const QString& src, const QString& dest )
{
    const KUrl u( src );
    const KUrl d( dest );

    // copy the file with file_copy
    KIO::Job* job = KIO::file_copy(u, d, -1, KIO::HideProgressInfo );
    job->setUiDelegate(0);
    bool ok = KIO::NetAccess::synchronousRun(job, 0);
    QVERIFY( ok );
    QVERIFY( QFile::exists( dest ) );
    QVERIFY( QFile::exists( src ) ); // still there

    {
        // check that the timestamp is the same (#24443)
        // Note: this only works because of copy() in kio_file.
        // The datapump solution ignores mtime, the app has to call FileCopyJob::setModificationTime()
        QFileInfo srcInfo( src );
        QFileInfo destInfo( dest );
#ifdef Q_WS_WIN
        // win32 time may differs in msec part
        QCOMPARE( srcInfo.lastModified().toString("dd.MM.yyyy hh:mm"),
                  destInfo.lastModified().toString("dd.MM.yyyy hh:mm") );
#else
        QCOMPARE( srcInfo.lastModified(), destInfo.lastModified() );
#endif
    }

    // cleanup and retry with KIO::copy()
    QFile::remove( dest );
    job = KIO::copy(u, d, KIO::HideProgressInfo );
    QSignalSpy spyCopyingDone(job, SIGNAL(copyingDone(KIO::Job*,const KUrl&,const KUrl&,time_t,bool,bool)));
    job->setUiDelegate(0);
    ok = KIO::NetAccess::synchronousRun(job, 0);
    QVERIFY( ok );
    QVERIFY( QFile::exists( dest ) );
    QVERIFY( QFile::exists( src ) ); // still there
    {
        // check that the timestamp is the same (#24443)
        QFileInfo srcInfo( src );
        QFileInfo destInfo( dest );
#ifdef Q_WS_WIN
        // win32 time may differs in msec part
        QCOMPARE( srcInfo.lastModified().toString("dd.MM.yyyy hh:mm"),
                  destInfo.lastModified().toString("dd.MM.yyyy hh:mm") );
#else
        QCOMPARE( srcInfo.lastModified(), destInfo.lastModified() );
#endif
    }
    QCOMPARE(spyCopyingDone.count(), 1);
}

void JobTest::copyLocalDirectory( const QString& src, const QString& _dest, int flags )
{
    QVERIFY( QFileInfo( src ).isDir() );
    QVERIFY( QFileInfo( src + "/testfile" ).isFile() );
    KUrl u;
    u.setPath( src );
    QString dest( _dest );
    KUrl d;
    d.setPath( dest );
    if ( flags & AlreadyExists )
        QVERIFY( QFile::exists( dest ) );
    else
        QVERIFY( !QFile::exists( dest ) );

    KIO::Job* job = KIO::copy(u, d, KIO::HideProgressInfo);
    job->setUiDelegate(0);
    bool ok = KIO::NetAccess::synchronousRun(job, 0);
    QVERIFY( ok );
    QVERIFY( QFile::exists( dest ) );
    QVERIFY( QFileInfo( dest ).isDir() );
    QVERIFY( QFileInfo( dest + "/testfile" ).isFile() );
    QVERIFY( QFile::exists( src ) ); // still there

    if ( flags & AlreadyExists ) {
        dest += '/' + u.fileName();
        //kDebug() << "Expecting dest=" << dest;
    }

    // CopyJob::setNextDirAttribute isn't implemented for Windows currently.
#ifndef Q_WS_WIN
    {
        // Check that the timestamp is the same (#24443)
        QFileInfo srcInfo( src );
        QFileInfo destInfo( dest );
        QCOMPARE( srcInfo.lastModified(), destInfo.lastModified() );
    }
#endif

    // Do it again, with Overwrite.
    // Use copyAs, we don't want a subdir inside d.
    job = KIO::copyAs(u, d, KIO::HideProgressInfo | KIO::Overwrite);
    job->setUiDelegate(0);
    ok = KIO::NetAccess::synchronousRun(job, 0);
    QVERIFY( ok );

    // Do it again, without Overwrite (should fail).
    job = KIO::copyAs(u, d, KIO::HideProgressInfo);
    job->setUiDelegate(0);
    ok = KIO::NetAccess::synchronousRun(job, 0);
    QVERIFY( !ok );
}

void JobTest::copyFileToSamePartition()
{
    kDebug() ;
    const QString filePath = homeTmpDir() + "fileFromHome";
    const QString dest = homeTmpDir() + "fileFromHome_copied";
    createTestFile( filePath );
    copyLocalFile( filePath, dest );
}

void JobTest::copyDirectoryToSamePartition()
{
    kDebug() ;
    const QString src = homeTmpDir() + "dirFromHome";
    const QString dest = homeTmpDir() + "dirFromHome_copied";
    createTestDirectory( src );
    copyLocalDirectory( src, dest );
}

void JobTest::copyDirectoryToExistingDirectory()
{
    kDebug() ;
    // just the same as copyDirectoryToSamePartition, but this time dest exists.
    // So we get a subdir, "dirFromHome_copy/dirFromHome"
    const QString src = homeTmpDir() + "dirFromHome";
    const QString dest = homeTmpDir() + "dirFromHome_copied";
    createTestDirectory( src );
    createTestDirectory( dest );
    copyLocalDirectory( src, dest, AlreadyExists );
}

void JobTest::copyFileToOtherPartition()
{
    kDebug() ;
    const QString filePath = homeTmpDir() + "fileFromHome";
    const QString dest = otherTmpDir() + "fileFromHome_copied";
    createTestFile( filePath );
    copyLocalFile( filePath, dest );
}

void JobTest::copyDirectoryToOtherPartition()
{
    kDebug() ;
    const QString src = homeTmpDir() + "dirFromHome";
    const QString dest = otherTmpDir() + "dirFromHome_copied";
    createTestDirectory( src );
    copyLocalDirectory( src, dest );
}

void JobTest::moveLocalFile( const QString& src, const QString& dest )
{
    QVERIFY( QFile::exists( src ) );
    KUrl u;
    u.setPath( src );
    KUrl d;
    d.setPath( dest );

    // move the file with file_move
    KIO::Job* job = KIO::file_move(u, d, -1, KIO::HideProgressInfo);
    job->setUiDelegate( 0 );
    bool ok = KIO::NetAccess::synchronousRun(job, 0);
    QVERIFY( ok );
    QVERIFY( QFile::exists( dest ) );
    QVERIFY( !QFile::exists( src ) ); // not there anymore

    // move it back with KIO::move()
    job = KIO::move( d, u, KIO::HideProgressInfo );
    job->setUiDelegate( 0 );
    ok = KIO::NetAccess::synchronousRun(job, 0);
    QVERIFY( ok );
    QVERIFY( !QFile::exists( dest ) );
    QVERIFY( QFile::exists( src ) ); // it's back
}

static void moveLocalSymlink( const QString& src, const QString& dest )
{
    KDE_struct_stat buf;
    QVERIFY ( KDE_lstat( QFile::encodeName( src ), &buf ) == 0 );
    KUrl u;
    u.setPath( src );
    KUrl d;
    d.setPath( dest );

    // move the symlink with move, NOT with file_move
    KIO::Job* job = KIO::move( u, d, KIO::HideProgressInfo );
    job->setUiDelegate( 0 );
    bool ok = KIO::NetAccess::synchronousRun(job, 0);
    if ( !ok )
        kWarning() << KIO::NetAccess::lastError();
    QVERIFY( ok );
    QVERIFY ( KDE_lstat( QFile::encodeName( dest ), &buf ) == 0 );
    QVERIFY( !QFile::exists( src ) ); // not there anymore

    // move it back with KIO::move()
    job = KIO::move( d, u, KIO::HideProgressInfo );
    job->setUiDelegate( 0 );
    ok = KIO::NetAccess::synchronousRun(job, 0);
    QVERIFY( ok );
    QVERIFY ( KDE_lstat( QFile::encodeName( dest ), &buf ) != 0 ); // doesn't exist anymore
    QVERIFY ( KDE_lstat( QFile::encodeName( src ), &buf ) == 0 ); // it's back
}

void JobTest::moveLocalDirectory( const QString& src, const QString& dest )
{
    kDebug() << src << " " << dest;
    QVERIFY( QFile::exists( src ) );
    QVERIFY( QFileInfo( src ).isDir() );
    QVERIFY( QFileInfo( src + "/testfile" ).isFile() );
#ifndef Q_WS_WIN
    QVERIFY( QFileInfo( src + "/testlink" ).isSymLink() );
#endif
    KUrl u;
    u.setPath( src );
    KUrl d;
    d.setPath( dest );

    KIO::Job* job = KIO::move( u, d, KIO::HideProgressInfo );
    job->setUiDelegate( 0 );
    bool ok = KIO::NetAccess::synchronousRun(job, 0);
    QVERIFY( ok );
    QVERIFY( QFile::exists( dest ) );
    QVERIFY( QFileInfo( dest ).isDir() );
    QVERIFY( QFileInfo( dest + "/testfile" ).isFile() );
    QVERIFY( !QFile::exists( src ) ); // not there anymore
#ifndef Q_WS_WIN
    QVERIFY( QFileInfo( dest + "/testlink" ).isSymLink() );
#endif
}

void JobTest::moveFileToSamePartition()
{
    kDebug() ;
    const QString filePath = homeTmpDir() + "fileFromHome";
    const QString dest = homeTmpDir() + "fileFromHome_moved";
    createTestFile( filePath );
    moveLocalFile( filePath, dest );
}

void JobTest::moveDirectoryToSamePartition()
{
    kDebug() ;
    const QString src = homeTmpDir() + "dirFromHome";
    const QString dest = homeTmpDir() + "dirFromHome_moved";
    createTestDirectory( src );
    moveLocalDirectory( src, dest );
}

void JobTest::moveFileToOtherPartition()
{
    kDebug() ;
    const QString filePath = homeTmpDir() + "fileFromHome";
    const QString dest = otherTmpDir() + "fileFromHome_moved";
    createTestFile( filePath );
    moveLocalFile( filePath, dest );
}

void JobTest::moveSymlinkToOtherPartition()
{
#ifndef Q_WS_WIN
    kDebug() ;
    const QString filePath = homeTmpDir() + "testlink";
    const QString dest = otherTmpDir() + "testlink_moved";
    createTestSymlink( filePath );
    moveLocalSymlink( filePath, dest );
#endif
}

void JobTest::moveDirectoryToOtherPartition()
{
    kDebug() ;
#ifndef Q_WS_WIN
    const QString src = homeTmpDir() + "dirFromHome";
    const QString dest = otherTmpDir() + "dirFromHome_moved";
    createTestDirectory( src );
    moveLocalDirectory( src, dest );
#endif
}

void JobTest::moveFileNoPermissions()
{
    kDebug() ;
#ifdef Q_WS_WIN
    kDebug() << "port to win32";
#else
    const QString src = "/etc/passwd";
    const QString dest = homeTmpDir() + "passwd";
    QVERIFY( QFile::exists( src ) );
    QVERIFY( QFileInfo( src ).isFile() );
    KUrl u;
    u.setPath( src );
    KUrl d;
    d.setPath( dest );

    KIO::CopyJob* job = KIO::move( u, d, KIO::HideProgressInfo );
    job->setUiDelegate( 0 ); // no skip dialog, thanks
    QMap<QString, QString> metaData;
    bool ok = KIO::NetAccess::synchronousRun( job, 0, 0, 0, &metaData );
    QVERIFY( !ok );
    QVERIFY( KIO::NetAccess::lastError() == KIO::ERR_ACCESS_DENIED );
    // OK this is fishy. Just like mv(1), KIO's behavior depends on whether
    // a direct rename(2) was used, or a full copy+del. In the first case
    // there is no destination file created, but in the second case the
    // destination file remains.
    //QVERIFY( QFile::exists( dest ) );
    QVERIFY( QFile::exists( src ) );
#endif
}

void JobTest::moveDirectoryNoPermissions()
{
    kDebug() ;
#ifdef Q_WS_WIN
    kDebug() << "port to win32";
#else

    // All of /etc is a bit much, so try to find something smaller:
    QString src = "/etc/fonts";
    if ( !QFile::exists( src ) )
        src = "/etc";

    const QString dest = homeTmpDir() + "mdnp";
    QVERIFY( QFile::exists( src ) );
    QVERIFY( QFileInfo( src ).isDir() );
    KUrl u;
    u.setPath( src );
    KUrl d;
    d.setPath( dest );

    KIO::CopyJob* job = KIO::move( u, d, KIO::HideProgressInfo );
    job->setUiDelegate( 0 ); // no skip dialog, thanks
    QMap<QString, QString> metaData;
    bool ok = KIO::NetAccess::synchronousRun( job, 0, 0, 0, &metaData );
    QVERIFY( !ok );
    QCOMPARE( KIO::NetAccess::lastError(), (int)KIO::ERR_ACCESS_DENIED );
    //QVERIFY( QFile::exists( dest ) ); // see moveFileNoPermissions
    QVERIFY( QFile::exists( src ) );
#endif
}

void JobTest::listRecursive()
{
    // Note: many other tests must have been run before since we rely on the files they created

    const QString src = homeTmpDir();
#ifndef Q_WS_WIN
    // Add a symlink to a dir, to make sure we don't recurse into those
    bool symlinkOk = symlink( "dirFromHome", QFile::encodeName( src + "/dirFromHome_link" ) ) == 0;
    QVERIFY( symlinkOk );
#endif
    KIO::ListJob* job = KIO::listRecursive( KUrl(src), KIO::HideProgressInfo );
    job->setUiDelegate( 0 );
    connect( job, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList& ) ),
             SLOT( slotEntries( KIO::Job*, const KIO::UDSEntryList& ) ) );
    bool ok = KIO::NetAccess::synchronousRun( job, 0 );
    QVERIFY( ok );
    m_names.sort();
    QByteArray ref_names = QByteArray( ".,..,"
            "dirFromHome,dirFromHome/testfile,"
#ifndef Q_WS_WIN
            "dirFromHome/testlink,"
#endif
            "dirFromHome_copied,"
            "dirFromHome_copied/dirFromHome,dirFromHome_copied/dirFromHome/testfile,"
#ifndef Q_WS_WIN
            "dirFromHome_copied/dirFromHome/testlink,"
#endif
            "dirFromHome_copied/testfile,"
#ifndef Q_WS_WIN
            "dirFromHome_copied/testlink,dirFromHome_link,"
#endif
            "fileFromHome,fileFromHome_copied");

    const QString joinedNames = m_names.join( "," );
    if (joinedNames.toLatin1() != ref_names) {
        qDebug( "%s", qPrintable( joinedNames ) );
        qDebug( "%s", ref_names.data() );
    }
    QCOMPARE( joinedNames.toLatin1(), ref_names );
}

void JobTest::listFile()
{
    const QString filePath = homeTmpDir() + "fileFromHome";
    createTestFile( filePath );
    KIO::ListJob* job = KIO::listDir(KUrl(filePath), KIO::HideProgressInfo);
    job->setUiDelegate( 0 );
    QVERIFY(!job->exec());
    QCOMPARE(job->error(), static_cast<int>(KIO::ERR_IS_FILE));

    // And list something that doesn't exist
    const QString path = homeTmpDir() + "fileFromHomeDoesNotExist";
    job = KIO::listDir(KUrl(path), KIO::HideProgressInfo);
    job->setUiDelegate( 0 );
    QVERIFY(!job->exec());
    QCOMPARE(job->error(), static_cast<int>(KIO::ERR_DOES_NOT_EXIST));
}

void JobTest::killJob()
{
    const QString src = homeTmpDir();
    KIO::ListJob* job = KIO::listDir( KUrl(src), KIO::HideProgressInfo );
    QVERIFY(job->isAutoDelete());
    QPointer<KIO::ListJob> ptr(job);
    job->setUiDelegate( 0 );
    qApp->processEvents(); // let the job start, it's no fun otherwise
    job->kill();
    qApp->sendPostedEvents(0, QEvent::DeferredDelete); // process the deferred delete of the job
    QVERIFY(ptr.isNull());
}

void JobTest::killJobBeforeStart()
{
    const QString src = homeTmpDir();
    KIO::Job* job = KIO::stat( KUrl(src), KIO::HideProgressInfo );
    QVERIFY(job->isAutoDelete());
    QPointer<KIO::Job> ptr(job);
    job->setUiDelegate( 0 );
    job->kill();
    qApp->sendPostedEvents(0, QEvent::DeferredDelete); // process the deferred delete of the job
    QVERIFY(ptr.isNull());
    qApp->processEvents(); // does KIO scheduler crash here? nope.
}

void JobTest::deleteJobBeforeStart() // #163171
{
    const QString src = homeTmpDir();
    KIO::Job* job = KIO::stat( KUrl(src), KIO::HideProgressInfo );
    QVERIFY(job->isAutoDelete());
    job->setUiDelegate( 0 );
    delete job;
    qApp->processEvents(); // does KIO scheduler crash here?
}

void JobTest::directorySize()
{
    // Note: many other tests must have been run before since we rely on the files they created

    const QString src = homeTmpDir();

    KIO::DirectorySizeJob* job = KIO::directorySize( KUrl(src) );
    job->setUiDelegate( 0 );
    bool ok = KIO::NetAccess::synchronousRun( job, 0 );
    QVERIFY( ok );
    kDebug() << "totalSize: " << job->totalSize();
    kDebug() << "totalFiles: " << job->totalFiles();
    kDebug() << "totalSubdirs: " << job->totalSubdirs();
#ifdef Q_WS_WIN
    QCOMPARE(job->totalFiles(), 5ULL); // see expected result in listRecursive() above
    QCOMPARE(job->totalSubdirs(), 3ULL); // see expected result in listRecursive() above
    QVERIFY(job->totalSize() > 54);
#else
    QCOMPARE(job->totalFiles(), 8ULL); // see expected result in listRecursive() above
    QCOMPARE(job->totalSubdirs(), 4ULL); // see expected result in listRecursive() above
    QVERIFY(job->totalSize() > 512);
#endif

    qApp->sendPostedEvents(0, QEvent::DeferredDelete);
}

void JobTest::directorySizeError()
{
    KIO::DirectorySizeJob* job = KIO::directorySize( KUrl("/I/Dont/Exist") );
    job->setUiDelegate( 0 );
    bool ok = KIO::NetAccess::synchronousRun( job, 0 );
    QVERIFY( !ok );
    qApp->sendPostedEvents(0, QEvent::DeferredDelete);
}

void JobTest::slotEntries( KIO::Job*, const KIO::UDSEntryList& lst )
{
    for( KIO::UDSEntryList::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
        QString displayName = (*it).stringValue( KIO::UDSEntry::UDS_NAME );
        //KUrl url = (*it).stringValue( KIO::UDSEntry::UDS_URL );
        m_names.append( displayName );
    }
}

#if 0 // old performance tests
class OldUDSAtom
{
public:
  QString m_str;
  long long m_long;
  unsigned int m_uds;
};
typedef QList<OldUDSAtom> OldUDSEntry; // well it was a QValueList :)

static void fillOldUDSEntry( OldUDSEntry& entry, time_t now_time_t, const QString& nameStr )
{
    OldUDSAtom atom;
    atom.m_uds = KIO::UDSEntry::UDS_NAME;
    atom.m_str = nameStr;
    entry.append( atom );
    atom.m_uds = KIO::UDSEntry::UDS_SIZE;
    atom.m_long = 123456ULL;
    entry.append( atom );
    atom.m_uds = KIO::UDSEntry::UDS_MODIFICATION_TIME;
    atom.m_long = now_time_t;
    entry.append( atom );
    atom.m_uds = KIO::UDSEntry::UDS_ACCESS_TIME;
    atom.m_long = now_time_t;
    entry.append( atom );
    atom.m_uds = KIO::UDSEntry::UDS_FILE_TYPE;
    atom.m_long = S_IFREG;
    entry.append( atom );
    atom.m_uds = KIO::UDSEntry::UDS_ACCESS;
    atom.m_long = 0644;
    entry.append( atom );
    atom.m_uds = KIO::UDSEntry::UDS_USER;
    atom.m_str = nameStr;
    entry.append( atom );
    atom.m_uds = KIO::UDSEntry::UDS_GROUP;
    atom.m_str = nameStr;
    entry.append( atom );
}

// QHash or QMap? doesn't seem to make much difference.
typedef QHash<uint, QVariant> UDSEntryHV;

static void fillUDSEntryHV( UDSEntryHV& entry, const QDateTime& now, const QString& nameStr )
{
    entry.reserve( 8 );
    entry.insert( KIO::UDSEntry::UDS_NAME, nameStr );
    // we might need a method to make sure people use unsigned long long
    entry.insert( KIO::UDSEntry::UDS_SIZE, 123456ULL );
    entry.insert( KIO::UDSEntry::UDS_MODIFICATION_TIME, now );
    entry.insert( KIO::UDSEntry::UDS_ACCESS_TIME, now );
    entry.insert( KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG );
    entry.insert( KIO::UDSEntry::UDS_ACCESS, 0644 );
    entry.insert( KIO::UDSEntry::UDS_USER, nameStr );
    entry.insert( KIO::UDSEntry::UDS_GROUP, nameStr );
}

// Which one is used depends on UDS_STRING vs UDS_LONG
struct UDSAtom4 // can't be a union due to qstring...
{
  UDSAtom4() {} // for QHash or QMap
  UDSAtom4( const QString& s ) : m_str( s ) {}
  UDSAtom4( long long l ) : m_long( l ) {}

  QString m_str;
  long long m_long;
};

// Another possibility, to save on QVariant costs
typedef QHash<uint, UDSAtom4> UDSEntryHS; // hash+struct

static void fillQHashStructEntry( UDSEntryHS& entry, time_t now_time_t, const QString& nameStr )
{
    entry.reserve( 8 );
    entry.insert( KIO::UDSEntry::UDS_NAME, nameStr );
    entry.insert( KIO::UDSEntry::UDS_SIZE, 123456ULL );
    entry.insert( KIO::UDSEntry::UDS_MODIFICATION_TIME, now_time_t );
    entry.insert( KIO::UDSEntry::UDS_ACCESS_TIME, now_time_t );
    entry.insert( KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG );
    entry.insert( KIO::UDSEntry::UDS_ACCESS, 0644 );
    entry.insert( KIO::UDSEntry::UDS_USER, nameStr );
    entry.insert( KIO::UDSEntry::UDS_GROUP, nameStr );
}

// Let's see if QMap makes any difference
typedef QMap<uint, UDSAtom4> UDSEntryMS; // map+struct

static void fillQMapStructEntry( UDSEntryMS& entry, time_t now_time_t, const QString& nameStr )
{
    entry.insert( KIO::UDSEntry::UDS_NAME, nameStr );
    entry.insert( KIO::UDSEntry::UDS_SIZE, 123456ULL );
    entry.insert( KIO::UDSEntry::UDS_MODIFICATION_TIME, now_time_t );
    entry.insert( KIO::UDSEntry::UDS_ACCESS_TIME, now_time_t );
    entry.insert( KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG );
    entry.insert( KIO::UDSEntry::UDS_ACCESS, 0644 );
    entry.insert( KIO::UDSEntry::UDS_USER, nameStr );
    entry.insert( KIO::UDSEntry::UDS_GROUP, nameStr );
}


void JobTest::newApiPerformance()
{
    const QDateTime now = QDateTime::currentDateTime();
    const time_t now_time_t = now.toTime_t();
    // use 30000 for callgrind, at least 100 times that for timing-based
    // use /10 times that in svn, so that jobtest doesn't last forever
    const int iterations = 30000 /* * 100 */ / 10;
    const int lookupIterations = 5 * iterations;
    const QString nameStr = QString::fromLatin1( "name" );

    /*
      This is to compare the old list-of-lists API vs a QMap/QHash-based API
      in terms of performance.

      The number of atoms and their type map to what kio_file would put in
      for any normal file.

      The lookups are done for two atoms that are present, and for one that is not.

    */

    /// Old API
    {
        qDebug( "Timing old api..." );

        // Slave code
        time_t start = time(0);
        for (int i = 0; i < iterations; ++i) {
            OldUDSEntry entry;
            fillOldUDSEntry( entry, now_time_t, nameStr );
        }
        qDebug("Old API: slave code: %ld", time(0) - start);

        OldUDSEntry entry;
        fillOldUDSEntry( entry, now_time_t, nameStr );
        QCOMPARE( entry.count(), 8 );

        start = time(0);

        // App code

        QString displayName;
        KIO::filesize_t size;
        KUrl url;


        for (int i = 0; i < lookupIterations; ++i) {
            OldUDSEntry::ConstIterator it2 = entry.begin();
            for( ; it2 != entry.end(); it2++ ) {
                switch ((*it2).m_uds) {
                case KIO::UDSEntry::UDS_NAME:
                    displayName = (*it2).m_str;
                    break;
                case KIO::UDSEntry::UDS_URL:
                    url = (*it2).m_str;
                    break;
                case KIO::UDSEntry::UDS_SIZE:
                    size = (*it2).m_long;
                    break;
                }
            }
        }

        qDebug("Old API: app code: %ld", time(0) - start);

        QCOMPARE( size, 123456ULL );
        QCOMPARE( displayName, QString::fromLatin1( "name" ) );
        QVERIFY( url.isEmpty() );
    }

    ///////// TEST CODE FOR FUTURE KIO API


    ////

    {
        qDebug( "Timing new QHash+QVariant api..." );

        // Slave code
        time_t start = time(0);
        for (int i = 0; i < iterations; ++i) {
            UDSEntryHV entry;
            fillUDSEntryHV( entry, now, nameStr );
        }

        qDebug("QHash+QVariant API: slave code: %ld", time(0) - start);

        UDSEntryHV entry;
        fillUDSEntryHV( entry, now, nameStr );
        QCOMPARE( entry.count(), 8 );

        start = time(0);

        // App code

        // Normally the code would look like this, but let's change it to time it like the old api
        /*
        QString displayName = entry.value( KIO::UDSEntry::UDS_NAME ).toString();
        KUrl url = entry.value( KIO::UDSEntry::UDS_URL ).toString();
        KIO::filesize_t size = entry.value( KIO::UDSEntry::UDS_SIZE ).toULongLong();
        */

        QString displayName;
        KIO::filesize_t size;
        KUrl url;

        for (int i = 0; i < lookupIterations; ++i) {

            // For a field that we assume to always be there
            displayName = entry.value( KIO::UDSEntry::UDS_NAME ).toString();

            // For a field that might not be there
            UDSEntryHV::const_iterator it = entry.find( KIO::UDSEntry::UDS_URL );
            const UDSEntryHV::const_iterator end = entry.end();
            if ( it != end )
                 url = it.value().toString();

            it = entry.find( KIO::UDSEntry::UDS_SIZE );
            if ( it != end )
                size = it.value().toULongLong();
        }

        qDebug("QHash+QVariant API: app code: %ld", time(0) - start);

        QCOMPARE( size, 123456ULL );
        QCOMPARE( displayName, QString::fromLatin1( "name" ) );
        QVERIFY( url.isEmpty() );
    }

    // ########### THE CHOSEN SOLUTION:
    {
        qDebug( "Timing new QHash+struct api..." );

        // Slave code
        time_t start = time(0);
        for (int i = 0; i < iterations; ++i) {
            UDSEntryHS entry;
            fillQHashStructEntry( entry, now_time_t, nameStr );
        }

        qDebug("QHash+struct API: slave code: %ld", time(0) - start);

        UDSEntryHS entry;
        fillQHashStructEntry( entry, now_time_t, nameStr );
        QCOMPARE( entry.count(), 8 );

        start = time(0);

        // App code

        QString displayName;
        KIO::filesize_t size;
        KUrl url;

        for (int i = 0; i < lookupIterations; ++i) {

            // For a field that we assume to always be there
            displayName = entry.value( KIO::UDSEntry::UDS_NAME ).m_str;

            // For a field that might not be there
            UDSEntryHS::const_iterator it = entry.find( KIO::UDSEntry::UDS_URL );
            const UDSEntryHS::const_iterator end = entry.end();
            if ( it != end )
                 url = it.value().m_str;

            it = entry.find( KIO::UDSEntry::UDS_SIZE );
            if ( it != end )
                size = it.value().m_long;
        }

        qDebug("QHash+struct API: app code: %ld", time(0) - start);

        QCOMPARE( size, 123456ULL );
        QCOMPARE( displayName, QString::fromLatin1( "name" ) );
        QVERIFY( url.isEmpty() );
    }

    {
        qDebug( "Timing new QMap+struct api..." );

        // Slave code
        time_t start = time(0);
        for (int i = 0; i < iterations; ++i) {
            UDSEntryMS entry;
            fillQMapStructEntry( entry, now_time_t, nameStr );
        }

        qDebug("QMap+struct API: slave code: %ld", time(0) - start);

        UDSEntryMS entry;
        fillQMapStructEntry( entry, now_time_t, nameStr );
        QCOMPARE( entry.count(), 8 );

        start = time(0);

        // App code

        QString displayName;
        KIO::filesize_t size;
        KUrl url;

        for (int i = 0; i < lookupIterations; ++i) {

            // For a field that we assume to always be there
            displayName = entry.value( KIO::UDSEntry::UDS_NAME ).m_str;

            // For a field that might not be there
            UDSEntryMS::const_iterator it = entry.find( KIO::UDSEntry::UDS_URL );
            const UDSEntryMS::const_iterator end = entry.end();
            if ( it != end )
                 url = it.value().m_str;

            it = entry.find( KIO::UDSEntry::UDS_SIZE );
            if ( it != end )
                size = it.value().m_long;
        }

        qDebug("QMap+struct API: app code: %ld", time(0) - start);

        QCOMPARE( size, 123456ULL );
        QCOMPARE( displayName, QString::fromLatin1( "name" ) );
        QVERIFY( url.isEmpty() );
    }
}
#endif

void JobTest::calculateRemainingSeconds()
{
    unsigned int seconds = KIO::calculateRemainingSeconds( 2 * 86400 - 60, 0, 1 );
    QCOMPARE( seconds, static_cast<unsigned int>( 2 * 86400 - 60 ) );
    QString text = KIO::convertSeconds( seconds );
    QCOMPARE( text, i18n( "1 day 23:59:00" ) );

    seconds = KIO::calculateRemainingSeconds( 520, 20, 10 );
    QCOMPARE( seconds, static_cast<unsigned int>( 50 ) );
    text = KIO::convertSeconds( seconds );
    QCOMPARE( text, i18n( "00:00:50" ) );
}

#if 0
void JobTest::copyFileToSystem()
{
    if ( !KProtocolInfo::isKnownProtocol( "system" ) ) {
        kDebug() << "no kio_system, skipping test";
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
    kDebug() << resolve_local_urls;
    extern KIO_EXPORT bool kio_resolve_local_urls;
    kio_resolve_local_urls = resolve_local_urls;

    const QString src = homeTmpDir() + "fileFromHome";
    createTestFile( src );
    KUrl u;
    u.setPath( src );
    KUrl d = systemTmpDir();
    d.addPath( "fileFromHome_copied" );

    kDebug() << "copying " << u << " to " << d;

    // copy the file with file_copy
    m_mimetype.clear();
    KIO::FileCopyJob* job = KIO::file_copy(u, d, -1, KIO::HideProgressInfo);
    job->setUiDelegate( 0 );
    connect( job, SIGNAL(mimetype(KIO::Job*,const QString&)),
             this, SLOT(slotMimetype(KIO::Job*,const QString&)) );
    bool ok = KIO::NetAccess::synchronousRun( job, 0 );
    QVERIFY( ok );

    QString dest = realSystemPath() + "fileFromHome_copied";

    QVERIFY( QFile::exists( dest ) );
    QVERIFY( QFile::exists( src ) ); // still there

    {
        // do NOT check that the timestamp is the same.
        // It can't work with file_copy when it uses the datapump,
        // unless we use setModificationTime in the app code.
    }

    // Check mimetype
    QCOMPARE(m_mimetype, QString("text/plain"));

    // cleanup and retry with KIO::copy()
    QFile::remove( dest );
    job = KIO::copy(u, d, KIO::HideProgressInfo);
    job->setUiDelegate(0);
    ok = KIO::NetAccess::synchronousRun(job, 0);
    QVERIFY( ok );
    QVERIFY( QFile::exists( dest ) );
    QVERIFY( QFile::exists( src ) ); // still there
    {
        // check that the timestamp is the same (#79937)
        QFileInfo srcInfo( src );
        QFileInfo destInfo( dest );
        QCOMPARE( srcInfo.lastModified(), destInfo.lastModified() );
    }

    // restore normal behavior
    kio_resolve_local_urls = true;
}
#endif

void JobTest::getInvalidUrl()
{
    KUrl url("http://strange<hostname>/");
    QVERIFY(!url.isValid());

    KIO::SimpleJob* job = KIO::get(url, KIO::NoReload, KIO::HideProgressInfo);
    QVERIFY(job != 0);
    job->setUiDelegate( 0 );

    KIO::Scheduler::scheduleJob(job); // shouldn't crash (#135456)

    bool ok = KIO::NetAccess::synchronousRun( job, 0 );
    QVERIFY( !ok ); // it should fail :)
}

void JobTest::slotMimetype(KIO::Job* job, const QString& type)
{
    QVERIFY( job != 0 );
    m_mimetype = type;
}

void JobTest::deleteFile()
{
    const QString dest = otherTmpDir() + "fileFromHome_copied";
    QVERIFY(QFile::exists(dest));
    KIO::Job* job = KIO::del(KUrl(dest), KIO::HideProgressInfo);
    job->setUiDelegate(0);
    bool ok = KIO::NetAccess::synchronousRun(job, 0);
    QVERIFY(ok);
    QVERIFY(!QFile::exists(dest));
}

void JobTest::deleteDirectory()
{
    const QString dest = otherTmpDir() + "dirFromHome_copied";
    if (!QFile::exists(dest))
        createTestDirectory(dest);
    // Let's put a few things in there to see if the recursive deletion works correctly
    // A hidden file:
    createTestFile(dest + "/.hidden");
#ifndef Q_WS_WIN
    // A broken symlink:
    createTestSymlink(dest+"/broken_symlink");
    // A symlink to a dir:
    bool symlink_ok = symlink( KDESRCDIR, QFile::encodeName( dest + "/symlink_to_dir" ) ) == 0;
    if ( !symlink_ok )
        kFatal() << "couldn't create symlink: " << strerror( errno ) ;
#endif

    KIO::Job* job = KIO::del(KUrl(dest), KIO::HideProgressInfo);
    job->setUiDelegate(0);
    bool ok = KIO::NetAccess::synchronousRun(job, 0);
    QVERIFY(ok);
    QVERIFY(!QFile::exists(dest));
}

void JobTest::deleteSymlink(bool using_fast_path)
{
    extern KIO_EXPORT bool kio_resolve_local_urls;
    kio_resolve_local_urls = !using_fast_path;

#ifndef Q_WS_WIN
    const QString src = homeTmpDir() + "dirFromHome";
    createTestDirectory(src);
    QVERIFY(QFile::exists(src));
    const QString dest = homeTmpDir() + "/dirFromHome_link";
    if (!QFile::exists(dest)) {
        // Add a symlink to a dir, to make sure we don't recurse into those
        bool symlinkOk = symlink(QFile::encodeName(src), QFile::encodeName(dest)) == 0;
        QVERIFY( symlinkOk );
        QVERIFY(QFile::exists(dest));
    }
    KIO::Job* job = KIO::del(KUrl(dest), KIO::HideProgressInfo);
    job->setUiDelegate(0);
    bool ok = KIO::NetAccess::synchronousRun(job, 0);
    QVERIFY(ok);
    QVERIFY(!QFile::exists(dest));
    QVERIFY(QFile::exists(src));
#endif

    kio_resolve_local_urls = true;
}


void JobTest::deleteSymlink()
{
#ifndef Q_WS_WIN
    deleteSymlink(true);
    deleteSymlink(false);
#endif
}

void JobTest::deleteManyDirs(bool using_fast_path)
{
    extern KIO_EXPORT bool kio_resolve_local_urls;
    kio_resolve_local_urls = !using_fast_path;

    const int numDirs = 50;
    KUrl::List dirs;
    for (int i = 0; i < numDirs; ++i) {
        const QString dir = homeTmpDir() + "dir" + QString::number(i);
        createTestDirectory(dir);
        dirs << KUrl(dir);
    }
    QTime dt;
    dt.start();
    KIO::Job* job = KIO::del(dirs, KIO::HideProgressInfo);
    job->setUiDelegate(0);
    bool ok = KIO::NetAccess::synchronousRun(job, 0);
    QVERIFY(ok);
    Q_FOREACH(const KUrl& dir, dirs) {
        QVERIFY(!QFile::exists(dir.path()));
    }

    kDebug() << "Deleted" << numDirs << "dirs in" << dt.elapsed() << "milliseconds";
    kio_resolve_local_urls = true;
}

void JobTest::deleteManyDirs()
{
    deleteManyDirs(true);
    deleteManyDirs(false);
}

static void createManyFiles(const QString& baseDir, int numFiles)
{
    for (int i = 0; i < numFiles; ++i) {
        // create empty file
        QFile f(baseDir + QString::number(i));
        QVERIFY(f.open(QIODevice::WriteOnly));
    }
}

void JobTest::deleteManyFilesIndependently()
{
    QTime dt;
    dt.start();
    const int numFiles = 100; // Use 1000 for performance testing
    const QString baseDir = homeTmpDir();
    createManyFiles(baseDir, numFiles);
    for (int i = 0; i < numFiles; ++i) {
        // delete each file independently. lots of jobs. this stress-tests kio scheduling.
        const QString file = baseDir + QString::number(i);
        QVERIFY(QFile::exists(file));
        //kDebug() << file;
        KIO::Job* job = KIO::del(KUrl(file), KIO::HideProgressInfo);
        job->setUiDelegate(0);
        bool ok = KIO::NetAccess::synchronousRun(job, 0);
        QVERIFY(ok);
        QVERIFY(!QFile::exists(file));
    }
    kDebug() << "Deleted" << numFiles << "files in" << dt.elapsed() << "milliseconds";
}

void JobTest::deleteManyFilesTogether(bool using_fast_path)
{
    extern KIO_EXPORT bool kio_resolve_local_urls;
    kio_resolve_local_urls = !using_fast_path;

    QTime dt;
    dt.start();
    const int numFiles = 100; // Use 1000 for performance testing
    const QString baseDir = homeTmpDir();
    createManyFiles(baseDir, numFiles);
    KUrl::List urls;
    for (int i = 0; i < numFiles; ++i) {
        const QString file = baseDir + QString::number(i);
        QVERIFY(QFile::exists(file));
        urls.append(KUrl(file));
    }

    //kDebug() << file;
    KIO::Job* job = KIO::del(urls, KIO::HideProgressInfo);
    job->setUiDelegate(0);
    bool ok = KIO::NetAccess::synchronousRun(job, 0);
    QVERIFY(ok);
    kDebug() << "Deleted" << numFiles << "files in" << dt.elapsed() << "milliseconds";

    kio_resolve_local_urls = true;
}

void JobTest::deleteManyFilesTogether()
{
    deleteManyFilesTogether(true);
    deleteManyFilesTogether(false);
}

void JobTest::rmdirEmpty()
{
    const QString dir = homeTmpDir() + "dir";
    QDir().mkdir(dir);
    QVERIFY(QFile::exists(dir));
    KIO::Job* job = KIO::rmdir(dir);
    QVERIFY(job->exec());
    QVERIFY(!QFile::exists(dir));
}

void JobTest::rmdirNotEmpty()
{
    const QString dir = homeTmpDir() + "dir";
    createTestDirectory(dir);
    createTestDirectory(dir + "/subdir");
    KIO::Job* job = KIO::rmdir(dir);
    QVERIFY(!job->exec());
    QVERIFY(QFile::exists(dir));
}

void JobTest::stat()
{
#if 1
    const QString filePath = homeTmpDir() + "fileFromHome";
    createTestFile( filePath );
    KIO::StatJob* job = KIO::stat(filePath, KIO::HideProgressInfo);
    QVERIFY(job);
    bool ok = KIO::NetAccess::synchronousRun(job, 0);
    QVERIFY(ok);
    // TODO set setSide, setDetails
    const KIO::UDSEntry& entry = job->statResult();
    QVERIFY(!entry.isDir());
    QVERIFY(!entry.isLink());
    QCOMPARE(entry.stringValue(KIO::UDSEntry::UDS_NAME), QString("fileFromHome"));
#else
    // Testing stat over HTTP
    KIO::StatJob* job = KIO::stat(KUrl("http://www.kde.org"), KIO::HideProgressInfo);
    QVERIFY(job);
    bool ok = KIO::NetAccess::synchronousRun(job, 0);
    QVERIFY(ok);
    // TODO set setSide, setDetails
    const KIO::UDSEntry& entry = job->statResult();
    QVERIFY(!entry.isDir());
    QVERIFY(!entry.isLink());
    QCOMPARE(entry.stringValue(KIO::UDSEntry::UDS_NAME), QString());
#endif
}

void JobTest::mostLocalUrl()
{
    const QString filePath = homeTmpDir() + "fileFromHome";
    createTestFile( filePath );
    KIO::StatJob* job = KIO::mostLocalUrl(filePath, KIO::HideProgressInfo);
    QVERIFY(job);
    bool ok = job->exec();
    QVERIFY(ok);
    QCOMPARE(job->mostLocalUrl().toLocalFile(), filePath);
}

void JobTest::mimeType()
{
#if 1
    const QString filePath = homeTmpDir() + "fileFromHome";
    createTestFile( filePath );
    KIO::MimetypeJob* job = KIO::mimetype(filePath, KIO::HideProgressInfo);
    QVERIFY(job);
    QSignalSpy spyMimeType(job, SIGNAL(mimetype(KIO::Job*, QString)));
    bool ok = KIO::NetAccess::synchronousRun(job, 0);
    QVERIFY(ok);
    QCOMPARE(spyMimeType.count(), 1);
    QCOMPARE(spyMimeType[0][0], QVariant::fromValue(static_cast<KIO::Job*>(job)));
    QCOMPARE(spyMimeType[0][1].toString(), QString("application/octet-stream"));
#else
    // Testing mimetype over HTTP
    KIO::MimetypeJob* job = KIO::mimetype(KUrl("http://www.kde.org"), KIO::HideProgressInfo);
    QVERIFY(job);
    QSignalSpy spyMimeType(job, SIGNAL(mimetype(KIO::Job*, QString)));
    bool ok = KIO::NetAccess::synchronousRun(job, 0);
    QVERIFY(ok);
    QCOMPARE(spyMimeType.count(), 1);
    QCOMPARE(spyMimeType[0][0], QVariant::fromValue(static_cast<KIO::Job*>(job)));
    QCOMPARE(spyMimeType[0][1].toString(), QString("text/html"));
#endif
}

void JobTest::moveFileDestAlreadyExists() // #157601
{
    const QString file1 = homeTmpDir() + "fileFromHome";
    createTestFile( file1 );
    const QString file2 = homeTmpDir() + "anotherFile";
    createTestFile( file2 );
    const QString existingDest = otherTmpDir() + "fileFromHome";
    createTestFile( existingDest );

    KUrl::List urls; urls << KUrl(file1) << KUrl(file2);
    KIO::CopyJob* job = KIO::move(urls, otherTmpDir(), KIO::HideProgressInfo);
    job->setUiDelegate(0);
    job->setAutoSkip(true);
    bool ok = KIO::NetAccess::synchronousRun(job, 0);
    QVERIFY(ok);
    QVERIFY(QFile::exists(file1)); // it was skipped
    QVERIFY(!QFile::exists(file2)); // it was moved
}

void JobTest::moveAndOverwrite()
{
    const QString sourceFile = homeTmpDir() + "fileFromHome";
    createTestFile( sourceFile );
    QString existingDest = otherTmpDir() + "fileFromHome";
    createTestFile( existingDest );

    KIO::FileCopyJob* job = KIO::file_move(KUrl(sourceFile), KUrl(existingDest), -1, KIO::HideProgressInfo | KIO::Overwrite);
    job->setUiDelegate(0);
    bool ok = KIO::NetAccess::synchronousRun(job, 0);
    QVERIFY(ok);
    QVERIFY(!QFile::exists(sourceFile)); // it was moved

#ifndef Q_WS_WIN
    // Now same thing when the target is a symlink to the source
    createTestFile( sourceFile );
    createTestSymlink( existingDest, QFile::encodeName(sourceFile) );
    QVERIFY(QFile::exists(existingDest));
    job = KIO::file_move(KUrl(sourceFile), KUrl(existingDest), -1, KIO::HideProgressInfo | KIO::Overwrite);
    job->setUiDelegate(0);
    ok = KIO::NetAccess::synchronousRun(job, 0);
    QVERIFY(ok);
    QVERIFY(!QFile::exists(sourceFile)); // it was moved

    // Now same thing when the target is a symlink to another file
    createTestFile( sourceFile );
    createTestFile( sourceFile + "2" );
    createTestSymlink( existingDest, QFile::encodeName(sourceFile + "2") );
    QVERIFY(QFile::exists(existingDest));
    job = KIO::file_move(KUrl(sourceFile), KUrl(existingDest), -1, KIO::HideProgressInfo | KIO::Overwrite);
    job->setUiDelegate(0);
    ok = KIO::NetAccess::synchronousRun(job, 0);
    QVERIFY(ok);
    QVERIFY(!QFile::exists(sourceFile)); // it was moved

    // Now same thing when the target is a _broken_ symlink
    createTestFile( sourceFile );
    createTestSymlink( existingDest );
    QVERIFY(!QFile::exists(existingDest)); // it exists, but it's broken...
    job = KIO::file_move(KUrl(sourceFile), KUrl(existingDest), -1, KIO::HideProgressInfo | KIO::Overwrite);
    job->setUiDelegate(0);
    ok = KIO::NetAccess::synchronousRun(job, 0);
    QVERIFY(ok);
    QVERIFY(!QFile::exists(sourceFile)); // it was moved
#endif
}

void JobTest::moveOverSymlinkToSelf() // #169547
{
#ifndef Q_WS_WIN
    const QString sourceFile = homeTmpDir() + "fileFromHome";
    createTestFile( sourceFile );
    const QString existingDest = homeTmpDir() + "testlink";
    createTestSymlink( existingDest, QFile::encodeName(sourceFile) );
    QVERIFY(QFile::exists(existingDest));

    KIO::CopyJob* job = KIO::move(KUrl(sourceFile), KUrl(existingDest), KIO::HideProgressInfo);
    job->setUiDelegate(0);
    bool ok = KIO::NetAccess::synchronousRun(job, 0);
    QVERIFY(!ok);
    QCOMPARE(job->error(), (int)KIO::ERR_FILE_ALREADY_EXIST); // and not ERR_IDENTICAL_FILES!
    QVERIFY(QFile::exists(sourceFile)); // it not moved
#endif
}

#include "jobtest.moc"
