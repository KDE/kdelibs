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

static const char* s_tarFileName = "karchivetest.tar.gz";

void KArchiveTest::testCreateTar()
{
    KTar tar( s_tarFileName );

    bool ok = tar.open( QIODevice::WriteOnly );
    QVERIFY( ok );

    ok = tar.writeFile( "empty", "weis", "users", "", 0 );
    QVERIFY( ok );
    ok = tar.writeFile( "test1", "weis", "users", "Hallo", 5 );
    QVERIFY( ok );
    ok = tar.writeFile( "test2", "weis", "users", "Hallo Du", 8 );
    QVERIFY( ok );
    ok = tar.writeFile( "mydir/test3", "weis", "users", "Noch so einer", 13 );
    QVERIFY( ok );
    ok = tar.writeFile( "my/dir/test3", "dfaure", "hackers", "I don't speak German (David)", 29 );
    QVERIFY( ok );

    static const int SIZE1 = 100;
    // Now a medium file : 100 null bytes
    char medium[ SIZE1 ];
    memset( medium, 0, SIZE1 );
    ok = tar.writeFile( "mediumfile", "user", "group", medium, SIZE1 );
    QVERIFY( ok );
    // Another one, with an absolute path
    ok = tar.writeFile( "/dir/subdir/mediumfile2", "user", "group", medium, SIZE1 );
    QVERIFY( ok );

    // Now a huge file : 20000 null bytes
    int n = 20000;
    char * huge = new char[ n ];
    memset( huge, 0, n );
    ok = tar.writeFile( "hugefile", "user", "group", huge, n );
    QVERIFY( ok );
    delete [] huge;

    ok = tar.close();
    QVERIFY( ok );

    QFileInfo fileInfo( QFile::encodeName( s_tarFileName ) );
    QVERIFY( fileInfo.exists() );
    QVERIFY( fileInfo.size() == 373 );
}

static QStringList recursiveListEntries( const KArchiveDirectory * dir, const QString & path )
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
    descr += QString("user=") + entry->user() + ' ';
    descr += QString("group=") + entry->group() + ' ';
    descr += QString("path=") + path+(*it) + ' ';
    descr += QString("type=") + ( entry->isDirectory() ? "dir" : "file" );
    if ( entry->isFile() )
        descr += QString(" size=") + QString::number( static_cast<const KArchiveFile *>(entry)->size() );
    //kDebug() << descr << endl;
    ret.append( descr );

    if (entry->isDirectory())
      ret += recursiveListEntries( (KArchiveDirectory *)entry, path+(*it)+"/" );
  }
  return ret;
}

void KArchiveTest::testReadTar()
{
    // testCreateTar must have been run first.

    KTar tar( s_tarFileName );

    bool ok = tar.open( QIODevice::ReadOnly );
    QVERIFY( ok );

    const KArchiveDirectory* dir = tar.directory();
    QVERIFY( dir != 0 );
    const QStringList listing = recursiveListEntries( dir, "" );

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
    QCOMPARE( listing[10], QString("mode=40755 user=weis group=users path=mydir/test3 type=file size=13") );
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
    const KArchiveDirectory* dir = tar.directory();

    const KArchiveEntry* e = dir->entry( "mydir/test3" );
    QVERIFY( e );
    QVERIFY( e->isFile() );
    const KArchiveFile* f = static_cast<const KArchiveFile*>( e );
    QByteArray arr( f->data() );
    kDebug() << k_funcinfo << arr << endl;
    QCOMPARE( arr.size(), 13 );
    QCOMPARE( arr, QByteArray( "Noch so einer" ) );

    // Now test using device()
    QIODevice *dev = f->device();
    QByteArray contents = dev->readAll();
    QCOMPARE( contents, arr );

    ok = tar.close();
    QVERIFY( ok );
}

void KArchiveTest::testMaxLength()
{
    KTar tar( s_tarFileName );

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
    const QStringList listing = recursiveListEntries( dir, "" );

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
