/* This file is part of the KDE project
   Copyright (C) 2006 David Faure <faure@kde.org>

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

// This file can only be included once in a given binary

#include <QtCore/qglobal.h>
#include <kde_file.h>
#ifdef Q_OS_UNIX
#include <utime.h>
#else
#include <sys/utime.h>
#endif
#include <errno.h>

QDateTime s_referenceTimeStamp;

static void setTimeStamp( const QString& path, const QDateTime& mtime )
{
#ifdef Q_OS_UNIX
    // Put timestamp in the past so that we can check that the listing is correct
    struct utimbuf utbuf;
    utbuf.actime = mtime.toTime_t();
    utbuf.modtime = utbuf.actime;
    utime( QFile::encodeName( path ), &utbuf );
    //qDebug( "Time changed for %s", qPrintable( path ) );
#elif defined(Q_OS_WIN)
    struct _utimbuf utbuf;
    utbuf.actime = mtime.toTime_t();
    utbuf.modtime = utbuf.actime;
    _wutime(reinterpret_cast<const wchar_t *>(path.utf16()), &utbuf);
#endif
}

static void createTestFile( const QString& path )
{
    QFile f( path );
    if ( !f.open( QIODevice::WriteOnly ) )
        kFatal() << "Can't create " << path;
    QByteArray data("Hello\0world", 11);
    QCOMPARE( data.size(), 11 );
    f.write(data);
    f.close();
    setTimeStamp( path, s_referenceTimeStamp );
}

static void createTestSymlink( const QString& path, const QByteArray& target = "/IDontExist" )
{
    QFile::remove(path);
    bool ok = symlink( target.constData(), QFile::encodeName( path ) ) == 0; // broken symlink
    if ( !ok )
        kFatal() << "couldn't create symlink: " << strerror( errno );
    KDE_struct_stat buf;
    QVERIFY( KDE_lstat( QFile::encodeName( path ), &buf ) == 0 );
    QVERIFY( S_ISLNK( buf.st_mode ) );
    //qDebug( "symlink %s created", qPrintable( path ) );
    QVERIFY( QFileInfo( path ).isSymLink() );
}

enum CreateTestDirectoryOptions { DefaultOptions = 0, NoSymlink = 1 };
static void createTestDirectory( const QString& path, CreateTestDirectoryOptions opt = DefaultOptions )
{
    QDir dir;
    bool ok = dir.mkdir( path );
    if ( !ok && !dir.exists() )
        kFatal() << "couldn't create " << path;
    createTestFile( path + "/testfile" );
#ifndef Q_WS_WIN
    if ( (opt & NoSymlink) == 0 ) {
        createTestSymlink( path + "/testlink" );
        QVERIFY( QFileInfo( path + "/testlink" ).isSymLink() );
    }
#else
    // to not change the filecount everywhere in the tests
    if ( (opt & NoSymlink) == 0 ) {
        createTestFile( path + "/testlink" );
    }
#endif
    setTimeStamp( path, s_referenceTimeStamp );
}

