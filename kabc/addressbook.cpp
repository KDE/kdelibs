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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <qtimer.h>
#include <qfile.h>
#include <qregexp.h>

#include <kapplication.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include "simpleformat.h"
#include "vcardformat.h"

#include "addressbook.h"
#include "addressbook.moc"

using namespace KABC;

AddressBook::AddressBook( Format *format )
{
  if ( !format ) {
    mFormat = new VCardFormat();
  } else {
    mFormat = format;
  }

  mFileCheckTimer = new QTimer( this );
  connect( mFileCheckTimer, SIGNAL( timeout() ), SLOT( checkFile() ) );
}

AddressBook::~AddressBook()
{
  delete mFormat;
}

bool AddressBook::load( const QString &fileName )
{
  setFileName( fileName );

  mAddressees.clear();

  return mFormat->load( this, fileName );
}

bool AddressBook::save( Ticket *ticket )
{
  kdDebug(5700) << "AddressBook::save() '" << ticket->fileName << "'" << endl;
  
  bool success = mFormat->save( this, ticket->fileName );

  setFileName( ticket->fileName );

  delete ticket;
  unlock( mFileName );

  return success;
}

bool AddressBook::reload()
{
  if ( fileName().isEmpty() ) return false;

  return load( fileName() );
}

void AddressBook::clear()
{
  mAddressees.clear();
}

AddressBook::Ticket *AddressBook::requestSaveTicket( const QString &fn )
{
  kdDebug(5700) << "AddressBook::requestSaveTicket(): '" << fn << "'" << endl; 

  QString saveFileName;

  if ( fn.isEmpty() ) saveFileName = fileName();
  else saveFileName = fn;

  if ( !lock( saveFileName ) ) {
    kdDebug() << "AddressBook::requestSaveTicket(): Can't lock file '"
              << saveFileName << "'" << endl;
    return 0;
  }
  return new Ticket( saveFileName );
}

void AddressBook::insertAddressee( const Addressee &a )
{
  Iterator it;
  for ( it = begin(); it != end(); ++it ) {
    if ( a.uid() == (*it).uid() ) {
      (*it) = a;
      return;
    }
  }
  mAddressees.append( a );
}

void AddressBook::removeAddressee( const Addressee &a )
{
  Iterator it;
  for ( it = begin(); it != end(); ++it ) {
    if ( a.uid() == (*it).uid() ) {
      removeAddressee( it );
      return;
    }
  }
}

void AddressBook::removeAddressee( const Iterator &it )
{
  mAddressees.remove( it.mIt );
}

AddressBook::Iterator AddressBook::find( const Addressee &a )
{
  Iterator it;
  for ( it = begin(); it != end(); ++it ) {
    if ( a.uid() == (*it).uid() ) {
      return it;
    }
  }
  return end();
}

Addressee AddressBook::findByUid( const QString &uid )
{
  Iterator it;
  for ( it = begin(); it != end(); ++it ) {
    if ( uid == (*it).uid() ) {
      return *it;
    }
  }
  return Addressee();
}

Addressee::List AddressBook::findByName( const QString &name )
{
  Addressee::List results;

  Iterator it;
  for ( it = begin(); it != end(); ++it ) {
    if ( name == (*it).name() ) {
      results.append( *it );
    }
  }

  return results;
}

Addressee::List AddressBook::findByEmail( const QString &email )
{
  Addressee::List results;

  Iterator it;
  for ( it = begin(); it != end(); ++it ) {
    if ( email == (*it).preferredEmail() ) {
      results.append( *it );
    }
  }

  return results;
}

Addressee::List AddressBook::findByCategory( const QString &category )
{
  Addressee::List results;

  Iterator it;
  for ( it = begin(); it != end(); ++it ) {
    if ( (*it).hasCategory( category) ) {
      results.append( *it );
    }
  }

  return results;
}

bool AddressBook::lock( const QString &fileName )
{
  kdDebug(5700) << "AddressBook::lock()" << endl;

  QString fn = fileName;
  fn.replace( QRegExp("/"), "_" );

  QString lockName = locateLocal( "data", "kabc/lock/" + fn + ".lock" );
  kdDebug(5700) << "-- lock name: " << lockName << endl;

  if (QFile::exists( lockName )) return false;

  QString lockUniqueName;
  lockUniqueName = fn + kapp->randomString(8);
  mLockUniqueName = locateLocal( "data", "kabc/lock/" + lockUniqueName );
  kdDebug(5700) << "-- lock unique name: " << mLockUniqueName << endl;

  // Create unique file
  QFile file( mLockUniqueName );
  file.open( IO_WriteOnly );
  file.close();

  // Create lock file
  int result = ::link( QFile::encodeName( mLockUniqueName ),
                       QFile::encodeName( lockName ) );

  if ( result == 0 ) {
    emit addressBookLocked( this );
    return true;
  }

  // TODO: check stat

  return false;
}

void AddressBook::unlock( const QString &fileName )
{
  QString fn = fileName;
  fn.replace( QRegExp( "/" ), "_" );

  QString lockName = locate( "data", "kabc/lock/" + fn + ".lock" );
  ::unlink( QFile::encodeName( lockName ) );
  QFile::remove( mLockUniqueName );
  emit addressBookUnlocked( this );
}

void AddressBook::setFileName( const QString &fileName )
{
  mFileName = fileName;

  struct stat s;
  int result = stat( QFile::encodeName( mFileName ), &s );
  if ( result == 0 ) {
    mChangeTime  = s.st_ctime;
  }

  mFileCheckTimer->start( 500 );
}

QString AddressBook::fileName() const
{
  return mFileName;
}

void AddressBook::checkFile()
{
  struct stat s;
  int result = stat( QFile::encodeName( mFileName ), &s );

#if 0
  kdDebug(5700) << "AddressBook::checkFile() result: " << result
            << " new ctime: " << s.st_ctime
            << " old ctime: " << mChangeTime
            << endl;
#endif

  if ( result == 0 && ( mChangeTime != s.st_ctime ) ) {
    mChangeTime  = s.st_ctime;
    load( mFileName );
    emit addressBookChanged( this );
  }
}

void AddressBook::dump() const
{
  kdDebug(5700) << "AddressBook::dump() --- begin ---" << endl;

  ConstIterator it;
  for( it = begin(); it != end(); ++it ) {
    (*it).dump();
  }

  kdDebug(5700) << "AddressBook::dump() ---  end  ---" << endl;
}
