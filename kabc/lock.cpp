/*
    This file is part of libkabc.
    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "lock.h"

#include <krandom.h>
#include <kinstance.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <ktempfile.h>

#include <qfile.h>
#include <qtextstream.h>

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

Lock::~Lock()
{
  unlock();
}

QString Lock::locksDir()
{
  return locateLocal( "data", "kabc/lock/" );
}

bool Lock::readLockFile( const QString &filename, int &pid, QString &app )
{
  QFile file( filename );
  if ( !file.open( QIODevice::ReadOnly ) ) return false;
  
  QTextStream t( &file );
  t >> pid >> ws >> app;

  return true;
}

bool Lock::writeLockFile( const QString &filename )
{
  QFile file( filename );
  if ( !file.open( QIODevice::WriteOnly ) ) return false;
  QTextStream t( &file );
  t << ::getpid() << endl << QString( KGlobal::instance()->instanceName() );

  return true;
}

QString Lock::lockFileName() const
{
  return locksDir() + mIdentifier + ".lock";
}

bool Lock::lock()
{
  kdDebug(5700) << "Lock::lock()" << endl;

  QString lockName = lockFileName();
  kdDebug(5700) << "-- lock name: " << lockName << endl;

  if ( QFile::exists( lockName ) ) {  // check if it is a stale lock file
    int pid;
    QString app;

    if ( !readLockFile( lockFileName(), pid, app ) ) {
      mError = i18n("Unable to open lock file.");
      return false;
    }

    int retval = ::kill( pid, 0 );
    if ( retval == -1 && errno == ESRCH ) { // process doesn't exists anymore
      QFile::remove( lockName );
      kdWarning(5700) << "Removed stale lock file from process '" << app << "'"
                      << endl;
    } else {
      QString identifier( mIdentifier );
      identifier.replace( '_', '/' );

      mError = i18n("The resource '%1' is locked by application '%2'.")
               .arg( identifier ).arg( app );
      return false;
    }
  }

  QString lockUniqueName;
  lockUniqueName = mIdentifier + KRandom::randomString( 8 );
  mLockUniqueName = locateLocal( "data", "kabc/lock/" + lockUniqueName );
  kdDebug(5700) << "-- lock unique name: " << mLockUniqueName << endl;

  // Create unique file
  writeLockFile( mLockUniqueName );

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
  int pid;
  QString app;
  if ( readLockFile( lockFileName(), pid, app ) ) {
    if ( pid == getpid() ) {
      QFile::remove( lockFileName() );
      QFile::remove( mLockUniqueName );
      emit unlocked();
    } else {
      mError = i18n("Unlock failed. Lock file is owned by other process: %1 (%2)")
               .arg( app ).arg( QString::number( pid ) );
      kdDebug() << "Lock::unlock(): " << mError << endl;
      return false;
    }
  }

  mError = "";
  return true;
}

QString Lock::error() const
{
  return mError;
}

#include "lock.moc"
