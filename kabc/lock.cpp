/*
    This file is part of libkabc.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include "lock.h"

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <ktempfile.h>

#include <qfile.h>

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace KABC;

Lock::Lock( const QString &identifier )
  : mIdentifier( identifier )
{
  mIdentifier.replace( "/", "_" );
}

bool Lock::lock()
{
  kdDebug(5700) << "Lock::lock()" << endl;

  QString lockName = locateLocal( "data", "kabc/lock/" + mIdentifier + ".lock" );
  kdDebug(5700) << "-- lock name: " << lockName << endl;

  if ( QFile::exists( lockName ) ) {  // check if it is a stale lock file
    QFile file( lockName );
    if ( !file.open( IO_ReadOnly ) ) {
      mError = i18n("Unable to open lock file.");
      return false;
    }

    QDataStream t( &file );

    QString app; int pid;
    t >> pid >> app;

    int retval = ::kill( pid, 0 );
    if ( retval == -1 && errno == ESRCH ) { // process doesn't exists anymore
      QFile::remove( lockName );
      kdError(5700) << "detect stale lock file from process '" << app << "'" << endl;
      file.close();
    } else {
      mError = i18n("The resource '%1' is locked by application '%2'.")
               .arg( mIdentifier ).arg( app );
      return false;
    }
  }

  QString lockUniqueName;
  lockUniqueName = mIdentifier + kapp->randomString( 8 );
  mLockUniqueName = locateLocal( "data", "kabc/lock/" + lockUniqueName );
  kdDebug(5700) << "-- lock unique name: " << mLockUniqueName << endl;

  // Create unique file
  QFile file( mLockUniqueName );
  file.open( IO_WriteOnly );
  QDataStream t( &file );
  t << ::getpid() << QString( KGlobal::instance()->instanceName() );
  file.close();

  // Create lock file
  int result = ::link( QFile::encodeName( mLockUniqueName ),
                       QFile::encodeName( lockName ) );

  if ( result == 0 ) {
    mError = "";
    emit locked();
    return true;
  }

  // TODO: check stat

  mError = i18n("Error");
  return false;
}

bool Lock::unlock()
{
  QString lockName = locateLocal( "data",
                                  "kabc/lock/" + mIdentifier + ".lock" );
  QFile::remove( lockName );
  QFile::remove( mLockUniqueName );
  emit unlocked();

  mError = "";
  return true;
}

QString Lock::error() const
{
  return mError;
}

#include "lock.moc"
