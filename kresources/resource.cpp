/*
    This file is part of libkresources.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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

#include <kdebug.h>
#include <kapplication.h>
#include <kconfig.h>

#include "resource.h"
#include "resource.moc"
using namespace KRES;

Resource::Resource( const KConfig* config ) :
  mOpenCount( 0 )
{
  if ( config ) {
    mType = config->readEntry( "ResourceType" );
    mName = config->readEntry( "ResourceName" );
    mReadOnly = config->readBoolEntry( "ResourceIsReadOnly" );
    mIdentifier = config->readEntry( "ResourceIdentifier" );
  } else {
    mType = "type";
    mName = "resource-name";
    mReadOnly = false;
    mIdentifier = KApplication::randomString( 10 );
  }
}

Resource::~Resource()
{
}

void Resource::writeConfig( KConfig* config )
{
  config->writeEntry( "ResourceType", mType );
  config->writeEntry( "ResourceName", mName );
  config->writeEntry( "ResourceIsReadOnly", mReadOnly );
  config->writeEntry( "ResourceIdentifier", mIdentifier );
}

bool Resource::open()
{
  bool result = true;
  mMutex.lock();
  if ( ! mOpenCount ) {
    kdDebug() << "Opening resource " << resourceName() << endl;
    result = doOpen();
  }
  mOpenCount++;
  mMutex.unlock();
  return result;
}

void Resource::close()
{
  mMutex.lock();
  if ( ! mOpenCount )
  {
    kdDebug() << "ERROR: Resource " << resourceName() << " closed more times than previously opened" << endl;
    mMutex.unlock();
    return;
  }
  mOpenCount--;
  if ( ! mOpenCount ) {
    kdDebug() << "Closing resource " << resourceName() << endl;
    doClose();
  } else {
    kdDebug() << "Not yet closing resource " << resourceName() << ", open count = " << mOpenCount << endl;
  }
  mMutex.unlock();
}

QString Resource::identifier() const
{
  return mIdentifier;
}

void Resource::setReadOnly( bool value )
{
  mReadOnly = value;
}

bool Resource::readOnly() const
{
  return mReadOnly;
}

void Resource::setResourceName( const QString &name )
{
  mName = name;
}

QString Resource::resourceName() const
{
  return mName;
}

QString Resource::encryptStr( const QString &str )
{
  QString result;
  for ( uint i = 0; i < str.length(); ++i )
    result += ( str[ i ].unicode() < 0x20 ) ? str[ i ] :
        QChar( 0x1001F - str[ i ].unicode() );

  return result;
}

QString Resource::decryptStr( const QString &str )
{
  // This encryption is symmetric
  return encryptStr( str );
}

