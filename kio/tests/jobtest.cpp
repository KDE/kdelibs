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
    kdDebug() << "All tests OK." << endl;
    return 0; // success. The exit(1) in check() is what happens in case of failure.
}

QString JobTest::homeTmpDir() const
{
    return QDir::homeDirPath() + "/.kde/jobtest/";
}

void JobTest::setup()
{
    // Start with a clean base dir
    KIO::NetAccess::del( homeTmpDir(), 0 );
    QDir dir; // TT: why not a static method?
    bool ok = dir.mkdir( homeTmpDir() );
    if ( !ok )
        kdFatal() << "Couldn't create " << homeTmpDir() << endl;
}

void JobTest::runAll()
{
    copyFileToFile();
}

static void createTestFile( const QString& path )
{
    QFile f( path );
    if ( !f.open( IO_WriteOnly ) )
        kdFatal() << "Can't create " << path << endl;
    f.writeBlock( "Hello world", 10 );
    f.close();
}

void JobTest::copyFileToFile()
{
    // setup
    QString filePath = homeTmpDir() + "fileFromHome";
    QString dest = homeTmpDir() + "destFile";
    createTestFile( filePath );
    KURL u;
    u.setPath( filePath );
    KURL d;
    d.setPath( dest );

    // test
    bool ok = KIO::NetAccess::file_copy( filePath, dest );
    assert( ok );
    assert( QFile::exists( dest ) );
    assert( QFile::exists( filePath ) ); // still there

    // cleanup and retry with copy()
    QFile::remove( dest );
    ok = KIO::NetAccess::copy( filePath, dest, 0 );
    assert( ok );
    assert( QFile::exists( dest ) );
    assert( QFile::exists( filePath ) ); // still there

}

