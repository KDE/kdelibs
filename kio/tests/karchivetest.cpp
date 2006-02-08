/*
 *  Copyright (C) 2006 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "karchivetest.h"
#include "karchivetest.moc"
#include <ktar.h>
#include <kzip.h>

#include <qtest_kde.h>
#include <qfileinfo.h>
#include <kdebug.h>

QTEST_KDEMAIN( KArchiveTest, NoGUI )

static const int SIZE1 = 100;

static void writeTestFilesToArchive( KArchive* archive )
{
    bool ok;
    ok = archive->writeFile( "empty", "weis", "users", "", 0 );
    QVERIFY( ok );
    ok = archive->writeFile( "test1", "weis", "users", "Hallo", 5 );
    QVERIFY( ok );
    ok = archive->writeFile( "test2", "weis", "users", "Hallo Du", 8 );
    QVERIFY( ok );
    QFile localFile( "test3" );
    ok = localFile.open( QIODevice::WriteOnly );
    QVERIFY( ok );
    ok = localFile.writeBlock( "Noch so einer", 13 ) == 13;
    QVERIFY( ok );
    localFile.close();
    ok = archive->addLocalFile( "test3", "mydir/test3" );
    QVERIFY( ok );
    ok = archive->writeFile( "my/dir/test3", "dfaure", "hackers", "I don't speak German (David)", 29 );
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
}

enum { WithUserGroup = 1 }; // ListingFlags

static QStringList recursiveListEntries( const KArchiveDirectory * dir, const QString & path, int listingFlags )
{
  QStringList ret;
  QStringList l = dir->entries();
  l.sort();
  QStringList::ConstIterator it = l.begin();
  for( ; it != l.end(); ++it )
  {
    const KArchiveEntry* entry = dir->entry( (*it) );

    QString descr;
    descr += QString("mode=") + QString::number( entry->permissions(), 8 ) + ' ';
    if ( listingFlags & WithUserGroup )
    {
        descr += QString("user=") + entry->user() + ' ';
        descr += QString("group=") + entry->group() + ' ';
    }
    descr += QString("path=") + path+(*it) + ' ';
    descr += QString("type=") + ( entry->isDirectory() ? "dir" : "file" );
    if ( entry->isFile() )
        descr += QString(" size=") + QString::number( static_cast<const KArchiveFile *>(entry)->size() );

    // TODO add date and time

    //kDebug() << descr << endl;
    ret.append( descr );

    if (entry->isDirectory())
      ret += recursiveListEntries( (KArchiveDirectory *)entry, path+(*it)+"/", listingFlags );
  }
  return ret;
}

static void testFileData( KArchive* archive )
{
    const KArchiveDirectory* dir = archive->directory();

    const KArchiveEntry* e = dir->entry( "mydir/test3" );
    QVERIFY( e );
    QVERIFY( e->isFile() );
    const KArchiveFile* f = static_cast<const KArchiveFile*>( e );
    QByteArray arr( f->data() );
    QCOMPARE( arr.size(), 13 );
    QCOMPARE( arr, QByteArray( "Noch so einer" ) );

    // Now test using device()
    QIODevice *dev = f->device();
    QByteArray contents = dev->readAll();
    QCOMPARE( contents, arr );

    e = dir->entry( "mediumfile" );
    QVERIFY( e && e->isFile() );
    f = (KArchiveFile*)e;
    QCOMPARE( f->data().size(), SIZE1 );

    e = dir->entry( "hugefile" );
    QVERIFY( e && e->isFile() );
    f = (KArchiveFile*)e;
    QCOMPARE( f->data().size(), 20000   );

}

static const char* s_tarFileName = "karchivetest.tar.gz";
static const char* s_tarMaxLengthFileName = "karchivetest-maxlength.tar.gz";
static const char* s_zipFileName = "karchivetest.zip";
static const char* s_zipMaxLengthFileName = "karchivetest-maxlength.zip";

void KArchiveTest::testCreateTar()
{
    KTar tar( s_tarFileName );

    bool ok = tar.open( QIODevice::WriteOnly );
    QVERIFY( ok );

    writeTestFilesToArchive( &tar );

    ok = tar.close();
    QVERIFY( ok );

    QFileInfo fileInfo( QFile::encodeName( s_tarFileName ) );
    QVERIFY( fileInfo.exists() );
    // We can't check for an exact size because of the addLocalFile, whose data is system-dependent
    QVERIFY( fileInfo.size() > 350 );
}

void KArchiveTest::testReadTar()
{
    // testCreateTar must have been run first.
    KTar tar( s_tarFileName );

    bool ok = tar.open( QIODevice::ReadOnly );
    QVERIFY( ok );

    const KArchiveDirectory* dir = tar.directory();
    QVERIFY( dir != 0 );
    const QStringList listing = recursiveListEntries( dir, "", WithUserGroup );

    QCOMPARE( listing.count(), 13 );
    QCOMPARE( listing[ 0], QString("mode=40777 user=dfaure group=dfaure path=dir type=dir") );
    QCOMPARE( listing[ 1], QString("mode=40777 user=dfaure group=dfaure path=dir/subdir type=dir") );
    QCOMPARE( listing[ 2], QString("mode=40755 user=user group=group path=dir/subdir/mediumfile2 type=file size=100") );
    QCOMPARE( listing[ 3], QString("mode=40755 user=weis group=users path=empty type=file size=0") );
    QCOMPARE( listing[ 4], QString("mode=40755 user=user group=group path=hugefile type=file size=20000") );
    QCOMPARE( listing[ 5], QString("mode=40755 user=user group=group path=mediumfile type=file size=100") );
    QCOMPARE( listing[ 6], QString("mode=40777 user=dfaure group=dfaure path=my type=dir") );
    QCOMPARE( listing[ 7], QString("mode=40777 user=dfaure group=dfaure path=my/dir type=dir") );
    QCOMPARE( listing[ 8], QString("mode=40755 user=dfaure group=hackers path=my/dir/test3 type=file size=29") );
    QCOMPARE( listing[ 9], QString("mode=40777 user=dfaure group=dfaure path=mydir type=dir") );
    // This one was added with addLocalFile, so ignore mode/user/group.
    QString str = listing[10];
    str.replace(QRegExp("mode.*path"), "path" );
    QCOMPARE( str, QString("path=mydir/test3 type=file size=13") );
    QCOMPARE( listing[11], QString("mode=40755 user=weis group=users path=test1 type=file size=5") );
    QCOMPARE( listing[12], QString("mode=40755 user=weis group=users path=test2 type=file size=8") );

    ok = tar.close();
    QVERIFY( ok );
}

void KArchiveTest::testTarFileData()
{
    // testCreateTar must have been run first.
    KTar tar( s_tarFileName );
    bool ok = tar.open( QIODevice::ReadOnly );
    QVERIFY( ok );

    testFileData( &tar );

    ok = tar.close();
    QVERIFY( ok );
}

void KArchiveTest::testTarMaxLength()
{
    KTar tar( s_tarMaxLengthFileName );

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

    QCOMPARE( listing[  0], QString("mode=40755 user=testu group=testg path=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa0000000098 type=file size=3") );
    QCOMPARE( listing[  3], QString("mode=40755 user=testu group=testg path=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa0000000101 type=file size=3") );
    QCOMPARE( listing[  4], QString("mode=40755 user=testu group=testg path=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa0000000102 type=file size=3") );

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

    QCOMPARE( listing.count(), 14 );
    QCOMPARE( listing[ 0], QString("mode=40777 path=dir type=dir") );
    QCOMPARE( listing[ 1], QString("mode=40777 path=dir/subdir type=dir") );
    QCOMPARE( listing[ 2], QString("mode=40755 path=dir/subdir/mediumfile2 type=file size=100") );
    QCOMPARE( listing[ 3], QString("mode=40755 path=empty type=file size=0") );
    QCOMPARE( listing[ 4], QString("mode=40755 path=hugefile type=file size=20000") );
    QCOMPARE( listing[ 5], QString("mode=40755 path=mediumfile type=file size=100") );
    QCOMPARE( listing[ 6], QString("mode=40755 path=mimetype type=file size=%1").arg(strlen(s_zipMimeType)) );
    QCOMPARE( listing[ 7], QString("mode=40777 path=my type=dir") );
    QCOMPARE( listing[ 8], QString("mode=40777 path=my/dir type=dir") );
    QCOMPARE( listing[ 9], QString("mode=40755 path=my/dir/test3 type=file size=29") );
    QCOMPARE( listing[10], QString("mode=40777 path=mydir type=dir") );
    // This one was added with addLocalFile, so ignore mode
    QString str = listing[11];
    str.replace(QRegExp("mode.*path"), "path" );
    QCOMPARE( str, QString("path=mydir/test3 type=file size=13") );
    QCOMPARE( listing[12], QString("mode=40755 path=test1 type=file size=5") );
    QCOMPARE( listing[13], QString("mode=40755 path=test2 type=file size=8") );

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

    QCOMPARE( listing[  0], QString("mode=40755 path=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa0000000098 type=file size=3") );
    QCOMPARE( listing[  3], QString("mode=40755 path=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa0000000101 type=file size=3") );
    QCOMPARE( listing[  4], QString("mode=40755 path=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa0000000102 type=file size=3") );

    QCOMPARE( listing.count(), 514 - 98 );

    ok = zip.close();
    QVERIFY( ok );
}

