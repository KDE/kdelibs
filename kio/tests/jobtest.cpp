/* This file is part of the KDE project
   Copyright (C) 2004 David Faure <faure@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "jobtest.h"

#include <config.h>

#include <kurl.h>
#include <kapplication.h>
#include <kio/netaccess.h>
#include <kdebug.h>
#include <kcmdlineargs.h>

#include <qfileinfo.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <qdir.h>

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
    KCmdLineArgs::init(argc,argv,"testtrash", 0, 0, 0, 0);
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
}

void JobTest::runAll()
{
    copyFileToSamePartition();
    copyDirectoryToSamePartition();
    copyFileToOtherPartition();
    copyDirectoryToOtherPartition();
}

void JobTest::cleanup()
{
    KIO::NetAccess::del( homeTmpDir(), 0 );
    KIO::NetAccess::del( otherTmpDir(), 0 );
}

static void createTestFile( const QString& path )
{
    QFile f( path );
    if ( !f.open( IO_WriteOnly ) )
        kdFatal() << "Can't create " << path << endl;
    f.writeBlock( "Hello world", 10 );
    f.close();
}

static void createTestDirectory( const QString& path )
{
    QDir dir;
    bool ok = dir.mkdir( path );
    if ( !ok )
        kdFatal() << "couldn't create " << path << endl;
    createTestFile( path + "/testfile" );
}

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

    // cleanup and retry with KIO::copy()
    QFile::remove( dest );
    ok = KIO::NetAccess::dircopy( u, d, 0 );
    assert( ok );
    assert( QFile::exists( dest ) );
    assert( QFile::exists( src ) ); // still there
}

void JobTest::copyLocalDirectory( const QString& src, const QString& dest )
{
    assert( QFileInfo( src ).isDir() );
    assert( QFileInfo( src + "/testfile" ).isFile() );
    KURL u;
    u.setPath( src );
    KURL d;
    d.setPath( dest );

    bool ok = KIO::NetAccess::dircopy( u, d, 0 );
    assert( ok );
    assert( QFile::exists( dest ) );
    assert( QFileInfo( dest ).isDir() );
    assert( QFileInfo( dest + "/testfile" ).isFile() );
    assert( QFile::exists( src ) ); // still there
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
