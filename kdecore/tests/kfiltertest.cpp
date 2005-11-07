/*
 *  Copyright (C) 2002-2005 David Faure   <faure@kde.org>
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

#include "QtTest/qttest_kde.h"

#include "kfilterdev.h"
#include "kfilterbase.h"
#include <unistd.h>
#include <limits.h>
#include <qfile.h>
#include <qtextstream.h>
#include <kdebug.h>
#include "kfiltertest.h"
#include <qdir.h>
#include <qfileinfo.h>

QTTEST_KDEMAIN(KFilterTest, NoGUI)

void KFilterTest::initTestCase()
{
    QString currentdir = QDir::currentPath();
    pathgz = currentdir + "/test.gz";
    pathbz2 = currentdir + "/test.bz2";

    // warning, update the COMPAREs in test_block_write() if changing the test data...
    testData = "hello world\n";
}

void KFilterTest::test_block_write( const QString & fileName )
{
    QIODevice * dev = KFilterDev::deviceForFile( fileName );
    VERIFY( dev != 0 );
    bool ok = dev->open( QIODevice::WriteOnly );
    VERIFY( ok );

    int ret = dev->write( testData );
    COMPARE( ret, testData.size() );

    dev->close();
    delete dev;

    VERIFY( QFile::exists( fileName ) );
}

void KFilterTest::test_block_write()
{
    kdDebug() << " -- test_block_write gzip -- " << endl;
    test_block_write(pathgz);
    VERIFY( QFileInfo( pathgz ).size() == 33 ); // size of test.gz

    kdDebug() << " -- test_block_write bzip2 -- " << endl;
    test_block_write(pathbz2);
    VERIFY( QFileInfo( pathbz2 ).size() == 52 ); // size of test.bz2
}

void KFilterTest::test_block_read( const QString & fileName )
{
    QIODevice * dev = KFilterDev::deviceForFile( fileName );
    VERIFY( dev != 0 );
    bool ok = dev->open( QIODevice::ReadOnly );
    VERIFY( ok );

    // This is what KGzipDev::readAll could do, if QIODevice::readAll was virtual....

    QByteArray array(1024);
    QByteArray read;
    int n;
    while ( ( n = dev->read( array.data(), array.size() ) ) )
    {
        VERIFY( n > 0 );
        read += QByteArray( array, n );
        //kdDebug() << "read returned " << n << endl;
        //kdDebug() << "read='" << read << "'" << endl;
        COMPARE( (int)dev->at(), (int)read.size() );
        //kdDebug() << "dev.at = " << dev->at() << endl;
    }
    COMPARE( read, testData );
    dev->close();
    delete dev;
}

void KFilterTest::test_block_read()
{
    kdDebug() << " -- test_block_read gzip -- " << endl;
    test_block_read(pathgz);
    kdDebug() << " -- test_block_read bzip2 -- " << endl;
    test_block_read(pathbz2);
}

void KFilterTest::test_getch( const QString & fileName )
{
    QIODevice * dev = KFilterDev::deviceForFile( fileName );
    VERIFY( dev != 0 );
    bool ok = dev->open( QIODevice::ReadOnly );
    VERIFY( ok );
    QByteArray read;
    int ch;
    while ( ( ch = dev->getch() ) != -1 ) {
        //printf("%c",ch);
        read += char( ch );
    }
    dev->close();
    delete dev;
    COMPARE( read, testData );
}

void KFilterTest::test_getch()
{
    kdDebug() << " -- test_getch gzip -- " << endl;
    test_getch(pathgz);
    kdDebug() << " -- test_getch bzip2 -- " << endl;
    test_getch(pathbz2);
}

void KFilterTest::test_textstream(  const QString & fileName )
{
    QIODevice * dev = KFilterDev::deviceForFile( fileName );
    VERIFY( dev != 0 );
    bool ok = dev->open( QIODevice::ReadOnly );
    VERIFY( ok );
    QTextStream ts( dev );
    QString readStr = ts.read();
    dev->close();
    delete dev;

    QByteArray read = readStr.toLatin1();
    COMPARE( read, testData );
}

void KFilterTest::test_textstream()
{
    kdDebug() << " -- test_textstream gzip -- " << endl;
    test_textstream(pathgz);
    kdDebug() << " -- test_textstream bzip2 -- " << endl;
    test_textstream(pathbz2);
}

#include "kfiltertest.moc"
