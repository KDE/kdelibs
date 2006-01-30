/*
  This file is part of the KDE libraries
  Copyright (c) 2006 Allen Winter <winter@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <QTextStream>

#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kcmdlineargs.h>
#include <ktempfile.h>
#include <ksavefile.h>

void test( const char *msg, bool result )
{
    fprintf( stderr, "Testing %s .... %s\n", msg, result ? "OK" : "FAILED" );
}

int main(int argc, char *argv[])
{
  KApplication::disableAutoDcopRegistration();
  KCmdLineArgs::init( argc, argv, "ksavefiletest", 0, 0, 0, 0 );
  KApplication app( false );

  KTempFile f( "fred", QString(), 0600 );
  test( "backup file", KSaveFile::backupFile( f.name() ) );
  test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
  test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
  test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
  test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
  test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
  test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
  test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
  test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
  test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
  test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
  test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
  test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
  test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
  test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
  test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
  test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
  //test( "numbered backup", KSaveFile::numberedBackupFile( f.name(),5 ) );
  test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
  test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
  test( "numbered backup", KSaveFile::numberedBackupFile( f.name() ) );
  //test( "numbered backup", KSaveFile::numberedBackupFile( f.name(),5 ) );
  test( "rcs backup", KSaveFile::rcsBackupFile( f.name() ) );
  test( "rcs backup", KSaveFile::rcsBackupFile( f.name() ) );
  // Test a change to f to verify RCS
#if 0  
  if ( f.textStream() ) { 
    QTextStream *out = f.textStream();
    //out->setEncoding( QTextStream::UnicodeUTF8 );
    (*out) << "Testing a change" << endl; 
    //*f.textStream() << "Testing a change" << endl; 
    f.close();
    test( "rcs backup", KSaveFile::rcsBackupFile( f.name(), QString(), "Testmsg" ) );
  }
#endif
}

