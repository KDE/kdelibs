/* This file is part of the KDE project
   Copyright (C) 2006, 2010 David Faure <faure@kde.org>

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


#include "karchivetest.h"
#include <kmimetype.h>
#include "karchivetest.moc"
#include <ktar.h>
#include <kzip.h>

#include <qtest_kde.h>
#include <QtCore/QFileInfo>
#include <kdebug.h>
#include <kfilterdev.h>
#include <ktempdir.h>
#ifndef Q_OS_WIN
#include <unistd.h> // symlink
#include <errno.h>
#endif

QTEST_KDEMAIN_CORE( KArchiveTest )

static const int SIZE1 = 100;

static void writeTestFilesToArchive( KArchive* archive )
{
    bool ok;
    ok = archive->writeFile( "empty", "weis", "users", "", 0 );
    QVERIFY( ok );
    ok = archive->writeFile( "test1", "weis", "users", "Hallo", 5, 0100440 );
    QVERIFY( ok );
    // Now let's try with the prepareWriting/writeData/finishWriting API
    ok = archive->prepareWriting( "test2", "weis", "users", 8 );
    QVERIFY( ok );
    ok = archive->writeData( "Hallo ", 6 );
    QVERIFY( ok );
    ok = archive->writeData( "Du", 2 );
    QVERIFY( ok );
    ok = archive->finishWriting( 8 );
    QVERIFY( ok );
    // Add local file
    QFile localFile( "test3" );
    ok = localFile.open( QIODevice::WriteOnly );
    QVERIFY( ok );
    ok = localFile.write( "Noch so einer", 13 ) == 13;
    QVERIFY( ok );
    localFile.close();
    ok = archive->addLocalFile( "test3", "z/test3" );
    QVERIFY( ok );

    // writeFile API
    ok = archive->writeFile( "my/dir/test3", "dfaure", "hackers", "I do not speak German\nDavid.", 29 );
    QVERIFY( ok );

    // Now a medium file : 100 null bytes
    char medium[ SIZE1 ];
    memset( medium, 0, SIZE1 );
    ok = archive->writeFile( "mediumfile", "user", "group", medium, SIZE1 );
    QVERIFY( ok );
    // Another one, with an absolute path
    ok = archive->writeFile( "/dir/subdir/mediumfile2", "user", "group", medium, SIZE1 );
    QVERIFY( ok );

    // Now a huge file : 20000 null bytes
    int n = 20000;
    char * huge = new char[ n ];
    memset( huge, 0, n );
    ok = archive->writeFile( "hugefile", "user", "group", huge, n );
    QVERIFY( ok );
    delete [] huge;

    // Now an empty directory
    ok = archive->writeDir( "aaaemptydir", "user", "group" );
    QVERIFY( ok );

#ifndef Q_OS_WIN
    // Add local symlink
    ok = archive->addLocalFile( "test3_symlink", "z/test3_symlink");
    QVERIFY( ok );
#endif
}

enum { WithUserGroup = 1 }; // ListingFlags

static QStringList recursiveListEntries( const KArchiveDirectory * dir, const QString & path, int listingFlags )
{
  QStringList ret;
  QStringList l = dir->entries();
  l.sort();
  Q_FOREACH(const QString& it, l) {
    const KArchiveEntry* entry = dir->entry(it);

    QString descr;
    descr += QString("mode=") + QString::number( entry->permissions(), 8 ) + ' ';
    if ( listingFlags & WithUserGroup )
    {
        descr += QString("user=") + entry->user() + ' ';
        descr += QString("group=") + entry->group() + ' ';
    }
    descr += QString("path=") + path+(it) + ' ';
    descr += QString("type=") + ( entry->isDirectory() ? "dir" : "file" );
    if ( entry->isFile() )
        descr += QString(" size=") + QString::number( static_cast<const KArchiveFile *>(entry)->size() );
    if (!entry->symLinkTarget().isEmpty())
        descr += QString(" symlink=") + entry->symLinkTarget();

    // TODO add date and time

    //kDebug() << descr;
    ret.append( descr );

    if (entry->isDirectory())
      ret += recursiveListEntries( (KArchiveDirectory *)entry, path+it+'/', listingFlags );
  }
  return ret;
}

static void testFileData( KArchive* archive )
{
    const KArchiveDirectory* dir = archive->directory();

    const KArchiveEntry* e = dir->entry( "z/test3" );
    QVERIFY( e );
    QVERIFY( e->isFile() );
    const KArchiveFile* f = static_cast<const KArchiveFile*>( e );
    QByteArray arr( f->data() );
    QCOMPARE( arr.size(), 13 );
    QCOMPARE( arr, QByteArray( "Noch so einer" ) );

    // Now test using createDevice()
    QIODevice *dev = f->createDevice();
    QByteArray contents = dev->readAll();
    QCOMPARE( contents, arr );
    delete dev;

    dev = f->createDevice();
    contents = dev->read(5); // test reading in two chunks
    QCOMPARE(contents.size(), 5);
    contents += dev->read(50);
    QCOMPARE(contents.size(), 13);
    QCOMPARE( QString::fromLatin1(contents), QString::fromLatin1(arr) );
    delete dev;

    e = dir->entry( "mediumfile" );
    QVERIFY( e && e->isFile() );
    f = (KArchiveFile*)e;
    QCOMPARE( f->data().size(), SIZE1 );

    e = dir->entry( "hugefile" );
    QVERIFY( e && e->isFile() );
    f = (KArchiveFile*)e;
    QCOMPARE( f->data().size(), 20000   );

    e = dir->entry( "aaaemptydir" );
    QVERIFY( e && e->isDirectory() );

    e = dir->entry( "my/dir/test3" );
    QVERIFY( e && e->isFile() );
    f = (KArchiveFile*)e;
    dev = f->createDevice();
    QByteArray firstLine = dev->readLine();
    QCOMPARE(QString::fromLatin1(firstLine), QString::fromLatin1("I do not speak German\n"));
    QByteArray secondLine = dev->read(100);
    QCOMPARE(QString::fromLatin1(secondLine), QString::fromLatin1("David."));
    delete dev;
#ifndef Q_OS_WIN
    e = dir->entry( "z/test3_symlink" );
    QVERIFY(e);
    QVERIFY(e->isFile());
    QCOMPARE(e->symLinkTarget(), QString("test3"));
#endif

    // Test "./" prefix for KOffice (xlink:href="./ObjectReplacements/Object 1")
    e = dir->entry( "./hugefile" );
    QVERIFY( e && e->isFile() );
    e = dir->entry( "./my/dir/test3" );
    QVERIFY( e && e->isFile() );

    // Test directory entries
    e = dir->entry( "my" );
    QVERIFY(e && e->isDirectory());
    e = dir->entry( "my/" );
    QVERIFY(e && e->isDirectory());
    e = dir->entry( "./my/" );
    QVERIFY(e && e->isDirectory());
}

static void testReadWrite( KArchive* archive )
{
    bool ok = archive->writeFile( "newfile", "dfaure", "users", "New File", 8, 0100440 );
    QVERIFY(ok);
}

static void testCopyTo( KArchive* archive )
{
    const KArchiveDirectory* dir = archive->directory();
    KTempDir tmpDir;
    const QString dirName = tmpDir.name();

    dir->copyTo( dirName );

    QVERIFY(QFile::exists(dirName+"dir"));
    QVERIFY(QFileInfo(dirName+"dir").isDir());

    QFileInfo fileInfo1(dirName+"dir/subdir/mediumfile2");
    QVERIFY(fileInfo1.exists());
    QVERIFY(fileInfo1.isFile());
    QCOMPARE(fileInfo1.size(), Q_INT64_C(100));

    QFileInfo fileInfo2(dirName+"hugefile");
    QVERIFY(fileInfo2.exists());
    QVERIFY(fileInfo2.isFile());
    QCOMPARE(fileInfo2.size(), Q_INT64_C(20000));

    QFileInfo fileInfo3(dirName+"mediumfile");
    QVERIFY(fileInfo3.exists());
    QVERIFY(fileInfo3.isFile());
    QCOMPARE(fileInfo3.size(), Q_INT64_C(100));

    QFileInfo fileInfo4(dirName+"my/dir/test3");
    QVERIFY(fileInfo4.exists());
    QVERIFY(fileInfo4.isFile());
    QCOMPARE(fileInfo4.size(), Q_INT64_C(29));

#ifndef Q_OS_WIN
    const QString fileName = dirName+"z/test3_symlink";
    const QFileInfo fileInfo5(fileName);
    QVERIFY(fileInfo5.exists());
    QVERIFY(fileInfo5.isFile());
    // Do not use fileInfo.readLink() for unix symlinks
    // It returns the -full- path to the target, while we want the target string "as is".
    QString symLinkTarget;
    const QByteArray encodedFileName = QFile::encodeName(fileName);
    QByteArray s;
#if defined(PATH_MAX)
    s.resize(PATH_MAX+1);
#else
    int path_max = pathconf(encodedFileName.data(), _PC_PATH_MAX);
    if (path_max <= 0) {
        path_max = 4096;
    }
    s.resize(path_max);
#endif
    int len = readlink(encodedFileName.data(), s.data(), s.size() - 1);
    if ( len >= 0 ) {
        s[len] = '\0';
        symLinkTarget = QFile::decodeName(s);
    }
    QCOMPARE(symLinkTarget, QString("test3"));
#endif
}

static const char s_tarFileName[] = "karchivetest.tar";
static const char s_tarGzFileName[]  = "karchivetest.tar.gz";
static const char s_tarBz2FileName[]  = "karchivetest.tar.bz2";
static const char s_tarGzMaxLengthFileName[] = "karchivetest-maxlength.tar.gz";
static const char s_zipFileName[] = "karchivetest.zip";
static const char s_zipMaxLengthFileName[] = "karchivetest-maxlength.zip";
static const char s_zipLocaleFileName[] = "karchivetest-locale.zip";

void KArchiveTest::initTestCase()
{
#ifndef Q_OS_WIN
    // Prepare local symlink
    QFile::remove("test3_symlink");
    if (::symlink("test3", "test3_symlink") != 0) {
        qDebug() << errno;
        QVERIFY(false);
    }
#endif

    // For better benchmarks: initialize KMimeTypeFactory magic here
    KMimeType::findByContent(QByteArray("hello"));
}

void KArchiveTest::testCreateTar()
{
    // With    tempfile: 0.7-0.8 ms, 994236 instr. loads
    // Without tempfile:    0.81 ms, 992541 instr. loads
    // Note: use ./karchivetest 2>&1 | grep ms
    //       to avoid being slowed down by the kDebugs.
    QBENCHMARK {

    KTar tar( s_tarFileName );

    bool ok = tar.open( QIODevice::WriteOnly );
    QVERIFY( ok );

    writeTestFilesToArchive( &tar );

    ok = tar.close();
    QVERIFY( ok );

    QFileInfo fileInfo( QFile::encodeName( s_tarFileName ) );
    QVERIFY( fileInfo.exists() );
    // We can't check for an exact size because of the addLocalFile, whose data is system-dependent
    QVERIFY( fileInfo.size() > 450 );
    }
}

void KArchiveTest::testCreateTarGz()
{
    // With    tempfile: 1.3-1.7 ms, 2555089 instr. loads
    // Without tempfile:    0.87 ms,  987915 instr. loads
    QBENCHMARK {

    KTar tar( s_tarGzFileName );

    bool ok = tar.open( QIODevice::WriteOnly );
    QVERIFY( ok );

    writeTestFilesToArchive( &tar );

    ok = tar.close();
    QVERIFY( ok );

    QFileInfo fileInfo( QFile::encodeName( s_tarGzFileName ) );
    QVERIFY( fileInfo.exists() );
    // We can't check for an exact size because of the addLocalFile, whose data is system-dependent
    QVERIFY( fileInfo.size() > 350 );

    }
}

void KArchiveTest::testCreateTarBz2()
{
    KTar tar( s_tarBz2FileName );

    bool ok = tar.open( QIODevice::WriteOnly );
    QVERIFY( ok );

    writeTestFilesToArchive( &tar );

    ok = tar.close();
    QVERIFY( ok );

    QFileInfo fileInfo( QFile::encodeName( s_tarBz2FileName ) );
    QVERIFY( fileInfo.exists() );
    // We can't check for an exact size because of the addLocalFile, whose data is system-dependent
    QVERIFY( fileInfo.size() > 350 );
}

void KArchiveTest::testReadTar() // testCreateTarGz must have been run first.
{
    kDebug() << "START";
    // 1.6-1.7 ms per interaction, 2908428 instruction loads
    // After the "no tempfile when writing fix" this went down
    // to 0.9-1.0 ms, 1689059 instruction loads.
    // I guess it finds the data in the kernel cache now that no KTempFile is
    // used when writing.
    QBENCHMARK {

    KTar tar( s_tarGzFileName );

    bool ok = tar.open( QIODevice::ReadOnly );
    QVERIFY( ok );

    const KArchiveDirectory* dir = tar.directory();
    QVERIFY( dir != 0 );
    const QStringList listing = recursiveListEntries( dir, "", WithUserGroup );

    QFileInfo localFileData("test3");

#ifndef Q_WS_WIN
    QCOMPARE( listing.count(), 15 );
#else
    QCOMPARE( listing.count(), 14 );
#endif
    QCOMPARE( listing[ 0], QString("mode=40755 user=user group=group path=aaaemptydir type=dir") );
    QCOMPARE( listing[ 1],
	      QString("mode=40777 user=%1 group=%2 path=dir type=dir").arg(localFileData.owner()).arg(localFileData.group()) );
    QCOMPARE( listing[ 2],
	      QString("mode=40777 user=%1 group=%2 path=dir/subdir type=dir").arg(localFileData.owner()).arg(localFileData.group()) );
    QCOMPARE( listing[ 3], QString("mode=100644 user=user group=group path=dir/subdir/mediumfile2 type=file size=100") );
    QCOMPARE( listing[ 4], QString("mode=100644 user=weis group=users path=empty type=file size=0") );
    QCOMPARE( listing[ 5], QString("mode=100644 user=user group=group path=hugefile type=file size=20000") );
    QCOMPARE( listing[ 6], QString("mode=100644 user=user group=group path=mediumfile type=file size=100") );
    QCOMPARE( listing[ 7], QString("mode=40777 user=%1 group=%2 path=my type=dir").arg(localFileData.owner()).arg(localFileData.group()) );
    QCOMPARE( listing[ 8], QString("mode=40777 user=%1 group=%2 path=my/dir type=dir").arg(localFileData.owner()).arg(localFileData.group()) );
    QCOMPARE( listing[ 9], QString("mode=100644 user=dfaure group=hackers path=my/dir/test3 type=file size=29") );
    QCOMPARE( listing[10], QString("mode=100440 user=weis group=users path=test1 type=file size=5") );
    QCOMPARE( listing[11], QString("mode=100644 user=weis group=users path=test2 type=file size=8") );
    QCOMPARE( listing[12], QString("mode=40777 user=%1 group=%2 path=z type=dir").arg(localFileData.owner()).arg(localFileData.group()) );
    // This one was added with addLocalFile, so ignore mode/user/group.
    QString str = listing[13];
    str.replace(QRegExp("mode.*path"), "path" );
    QCOMPARE( str, QString("path=z/test3 type=file size=13") );
#ifndef Q_OS_WIN
    str = listing[14];
    str.replace(QRegExp("mode.*path"), "path" );
    QCOMPARE( str, QString("path=z/test3_symlink type=file size=0 symlink=test3") );
#endif

    ok = tar.close();
    QVERIFY( ok );
    }
}

// This tests the decompression using kfilterdev, basically.
// To debug KTarPrivate::fillTempFile().
void KArchiveTest::testUncompress()
{
    // testCreateTar must have been run first.
    QVERIFY( QFile::exists( s_tarGzFileName ) );
    QIODevice *filterDev = KFilterDev::deviceForFile( s_tarGzFileName, "application/x-gzip", true );
    QVERIFY( filterDev );
    QByteArray buffer;
    buffer.resize(8*1024);
    kDebug() << "buffer.size()=" << buffer.size();
    QVERIFY( filterDev->open( QIODevice::ReadOnly ) );

    qint64 totalSize = 0;
    qint64 len = -1;
    while ( !filterDev->atEnd() && len != 0 ) {
        len = filterDev->read(buffer.data(), buffer.size());
        QVERIFY( len >= 0 );
        totalSize += len;
        // kDebug() << "read len=" << len << " totalSize=" << totalSize;
    }
    filterDev->close();
    delete filterDev;
    // kDebug() << "totalSize=" << totalSize;
    QVERIFY( totalSize > 26000 ); // 27648 here when using gunzip
}

void KArchiveTest::testTarFileData()
{
    // testCreateTar must have been run first.
    KTar tar( s_tarGzFileName );
    bool ok = tar.open( QIODevice::ReadOnly );
    QVERIFY( ok );

    testFileData( &tar );

    ok = tar.close();
    QVERIFY( ok );
}

void KArchiveTest::testTarCopyTo()
{
    // testCreateTar must have been run first.
    KTar tar( s_tarGzFileName );
    bool ok = tar.open( QIODevice::ReadOnly );
    QVERIFY( ok );

    testCopyTo( &tar );

    ok = tar.close();
    QVERIFY( ok );
}

void KArchiveTest::testTarReadWrite()
{
    // testCreateTar must have been run first.
    KTar tar( s_tarGzFileName );
    bool ok = tar.open( QIODevice::ReadWrite );
    QVERIFY( ok );

    testReadWrite( &tar );
    testFileData( &tar );

    ok = tar.close();
    QVERIFY( ok );

    // Reopen it and check it
    {
        KTar tar( s_tarGzFileName );
        bool ok = tar.open( QIODevice::ReadOnly );
        QVERIFY( ok );
        testFileData( &tar );
        const KArchiveDirectory* dir = tar.directory();
        const KArchiveEntry* e = dir->entry( "newfile" );
        QVERIFY( e && e->isFile() );
        const KArchiveFile* f = (KArchiveFile*)e;
        QCOMPARE( f->data().size(), 8 );
    }
}

void KArchiveTest::testTarMaxLength()
{
    KTar tar( s_tarGzMaxLengthFileName );

    bool ok = tar.open( QIODevice::WriteOnly );
    QVERIFY( ok );

    // Generate long filenames of each possible length bigger than 98...
    // Also exceed 512 byte block size limit to see how well the ././@LongLink
    // implementation fares
    for (int i = 98; i < 514 ; i++ )
    {
      QString str, num;
      str.fill( 'a', i-10 );
      num.setNum( i );
      num = num.rightJustified( 10, '0' );
      tar.writeFile( str+num, "testu", "testg", "hum", 3 );
    }
    // Result of this test : works perfectly now (failed at 482 formerly and
    // before that at 154).
    ok = tar.close();
    QVERIFY( ok );

    ok = tar.open( QIODevice::ReadOnly );
    QVERIFY( ok );

    const KArchiveDirectory* dir = tar.directory();
    QVERIFY( dir != 0 );
    const QStringList listing = recursiveListEntries( dir, "", WithUserGroup );

    QCOMPARE( listing[  0], QString("mode=100644 user=testu group=testg path=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa0000000098 type=file size=3") );
    QCOMPARE( listing[  3], QString("mode=100644 user=testu group=testg path=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa0000000101 type=file size=3") );
    QCOMPARE( listing[  4], QString("mode=100644 user=testu group=testg path=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa0000000102 type=file size=3") );

    // TODO:
    // ################################################# BUG! ###########################
    // There seems to be a bug (which is in kde3 too), we miss 512 and 513.
    // But note that tar tvzf says "skipping next header" (and it skips 511),
    // so the bug is probably during writing...
    QCOMPARE( listing.count(), /*514 - 98*/ 414 );

    ok = tar.close();
    QVERIFY( ok );
}

///

static const char* s_zipMimeType = "application/vnd.oasis.opendocument.text";

void KArchiveTest::testCreateZip()
{
    KZip zip( s_zipFileName );

    bool ok = zip.open( QIODevice::WriteOnly );
    QVERIFY( ok );

    zip.setExtraField( KZip::NoExtraField );

    zip.setCompression( KZip::NoCompression );
    QByteArray zipMimeType( s_zipMimeType );
    zip.writeFile( "mimetype", "", "", zipMimeType.data(), zipMimeType.size() );
    zip.setCompression( KZip::DeflateCompression );

    writeTestFilesToArchive( &zip );

    ok = zip.close();
    QVERIFY( ok );

    QFile zipFile( QFile::encodeName( s_zipFileName ) );
    QFileInfo fileInfo( zipFile );
    QVERIFY( fileInfo.exists() );
    QVERIFY( fileInfo.size() > 300 );

    // Check that the header with no-compression and no-extrafield worked.
    // (This is for the "magic" for koffice documents)
    ok = zipFile.open( QIODevice::ReadOnly );
    QVERIFY( ok );
    QByteArray arr = zipFile.read( 4 );
    QCOMPARE( arr, QByteArray( "PK\003\004" ) );
    ok = zipFile.seek( 30 );
    QVERIFY( ok );
    arr = zipFile.read( 8 );
    QCOMPARE( arr, QByteArray( "mimetype" ) );
    arr = zipFile.read( zipMimeType.size() );
    QCOMPARE( arr, zipMimeType );
}

void KArchiveTest::testCreateZipError()
{
    // Giving a directory name to kzip must give an error case in close(), see #136630.
    // Otherwise we just lose data.
    KZip zip( QDir::currentPath() );

    bool ok = zip.open( QIODevice::WriteOnly );
    QVERIFY( ok );

    writeTestFilesToArchive( &zip );

    ok = zip.close();
    QVERIFY( !ok );
}

void KArchiveTest::testReadZip()
{
    // testCreateZip must have been run first.
    KZip zip( s_zipFileName );

    bool ok = zip.open( QIODevice::ReadOnly );
    QVERIFY( ok );

    const KArchiveDirectory* dir = zip.directory();
    QVERIFY( dir != 0 );

    // ZIP has no support for per-file user/group, so omit them from the listing
    const QStringList listing = recursiveListEntries( dir, "", 0 );

#ifndef Q_WS_WIN
    QCOMPARE( listing.count(), 16 );
#else
    QCOMPARE( listing.count(), 15 );
#endif
    QCOMPARE( listing[ 0], QString("mode=40755 path=aaaemptydir type=dir") );
    QCOMPARE( listing[ 1], QString("mode=40777 path=dir type=dir") );
    QCOMPARE( listing[ 2], QString("mode=40777 path=dir/subdir type=dir") );
    QCOMPARE( listing[ 3], QString("mode=100644 path=dir/subdir/mediumfile2 type=file size=100") );
    QCOMPARE( listing[ 4], QString("mode=100644 path=empty type=file size=0") );
    QCOMPARE( listing[ 5], QString("mode=100644 path=hugefile type=file size=20000") );
    QCOMPARE( listing[ 6], QString("mode=100644 path=mediumfile type=file size=100") );
    QCOMPARE( listing[ 7], QString("mode=100644 path=mimetype type=file size=%1").arg(strlen(s_zipMimeType)) );
    QCOMPARE( listing[ 8], QString("mode=40777 path=my type=dir") );
    QCOMPARE( listing[ 9], QString("mode=40777 path=my/dir type=dir") );
    QCOMPARE( listing[10], QString("mode=100644 path=my/dir/test3 type=file size=29") );
    QCOMPARE( listing[11], QString("mode=100440 path=test1 type=file size=5") );
    QCOMPARE( listing[12], QString("mode=100644 path=test2 type=file size=8") );
    QCOMPARE( listing[13], QString("mode=40777 path=z type=dir") );
    // This one was added with addLocalFile, so ignore mode
    QString str = listing[14];
    str.replace(QRegExp("mode.*path"), "path" );
    QCOMPARE( str, QString("path=z/test3 type=file size=13") );
#ifndef Q_OS_WIN
    str = listing[15];
    str.replace(QRegExp("mode.*path"), "path" );
    QCOMPARE( str, QString("path=z/test3_symlink type=file size=5 symlink=test3") );
#endif

    ok = zip.close();
    QVERIFY( ok );
}

void KArchiveTest::testZipFileData()
{
    // testCreateZip must have been run first.
    KZip zip( s_zipFileName );
    bool ok = zip.open( QIODevice::ReadOnly );
    QVERIFY( ok );

    testFileData( &zip );

    ok = zip.close();
    QVERIFY( ok );
}

void KArchiveTest::testZipCopyTo()
{
    // testCreateZip must have been run first.
    KZip zip( s_zipFileName );
    bool ok = zip.open( QIODevice::ReadOnly );
    QVERIFY( ok );

    testCopyTo( &zip );

    ok = zip.close();
    QVERIFY( ok );
}

void KArchiveTest::testZipMaxLength()
{
    KZip zip( s_zipMaxLengthFileName );

    bool ok = zip.open( QIODevice::WriteOnly );
    QVERIFY( ok );

    // Similar to testTarMaxLength just to make sure, but of course zip doesn't have
    // those limitations in the first place.
    for (int i = 98; i < 514 ; i++ )
    {
      QString str, num;
      str.fill( 'a', i-10 );
      num.setNum( i );
      num = num.rightJustified( 10, '0' );
      zip.writeFile( str+num, "testu", "testg", "hum", 3 );
    }
    ok = zip.close();
    QVERIFY( ok );

    ok = zip.open( QIODevice::ReadOnly );
    QVERIFY( ok );

    const KArchiveDirectory* dir = zip.directory();
    QVERIFY( dir != 0 );
    const QStringList listing = recursiveListEntries( dir, "", 0 );

    QCOMPARE( listing[  0], QString("mode=100644 path=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa0000000098 type=file size=3") );
    QCOMPARE( listing[  3], QString("mode=100644 path=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa0000000101 type=file size=3") );
    QCOMPARE( listing[  4], QString("mode=100644 path=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa0000000102 type=file size=3") );

    QCOMPARE( listing.count(), 514 - 98 );

    ok = zip.close();
    QVERIFY( ok );
}

void KArchiveTest::testZipWithNonLatinFileNames()
{
    KZip zip( s_zipLocaleFileName );

    bool ok = zip.open( QIODevice::WriteOnly );
    QVERIFY( ok );

    const QByteArray fileData("Test of data with a russian file name");
    const QString fileName = QString::fromUtf8( "Архитектура.okular" );
    const QString recodedFileName = QFile::decodeName( QFile::encodeName( fileName ) );
    ok = zip.writeFile( fileName, "pino", "users", fileData.constData(), fileData.size() );
    QVERIFY( ok );

    ok = zip.close();
    QVERIFY( ok );

    ok = zip.open( QIODevice::ReadOnly );
    QVERIFY( ok );

    const KArchiveDirectory* dir = zip.directory();
    QVERIFY( dir != 0 );
    const QStringList listing = recursiveListEntries( dir, "", 0 );

    QCOMPARE( listing.count(), 1 );
    QCOMPARE( listing[0], QString::fromUtf8("mode=100644 path=%1 type=file size=%2").arg(recodedFileName).arg(fileData.size()) );

    const KArchiveFile* fileEntry = static_cast< const KArchiveFile* >( dir->entry( dir->entries()[0] ) );
    QCOMPARE( fileEntry->data(), fileData );
}

void KArchiveTest::cleanupTestCase()
{
    QFile::remove(s_tarGzMaxLengthFileName);
    QFile::remove(s_zipMaxLengthFileName);
    QFile::remove(s_tarGzFileName);
    QFile::remove(s_zipFileName);
    QFile::remove(s_zipLocaleFileName);
    QFile::remove(s_tarFileName);
#ifndef Q_OS_WIN
    QFile::remove("test3_symlink");
#endif
}

static bool writeFile(const QString& dirName, const QString& fileName, const QByteArray& data)
{
    Q_ASSERT(dirName.endsWith('/'));
    QFile file(dirName + fileName);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    file.write(data);
    return true;
}

void KArchiveTest::testZipAddLocalDirectory()
{
    // Prepare local dir
    KTempDir tmpDir;
    const QString dirName = tmpDir.name();

    const QByteArray file1Data = "Hello Shantanu";
    const QString file1 = QLatin1String("file1");
    QVERIFY(writeFile(dirName, file1, file1Data));

    {
        KZip zip(s_zipFileName);

        bool ok = zip.open(QIODevice::WriteOnly);
        QVERIFY(ok);
        ok = zip.addLocalDirectory(dirName, ".");
        QVERIFY(ok);
        ok = zip.close();
        QVERIFY(ok);
    }
    {
        KZip zip(s_zipFileName);

        bool ok = zip.open(QIODevice::ReadOnly);
        QVERIFY(ok);

        const KArchiveDirectory* dir = zip.directory();
        QVERIFY(dir != 0);

        const KArchiveEntry* e = dir->entry(file1 );
        QVERIFY(e && e->isFile());
        const KArchiveFile* f = (KArchiveFile*)e;
        QCOMPARE(f->data(), file1Data);
    }
}
