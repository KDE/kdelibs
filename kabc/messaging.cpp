/*
    This file is part of libkabc.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#include <kapplication.h>

#include <qdatastream.h>

#include "messaging.h"

using namespace KABC;

Messaging::Messaging()
{
  mId = KApplication::randomString( 8 );
}

Messaging::Messaging( const QString &serviceType )
  : mServiceType( serviceType )
{
  mId = KApplication::randomString( 8 );
}

void Messaging::setId( const QString &id )
{
  mId = id;
}

QString Messaging::id() const
{
  return mId;
}

void Messaging::setServiceType( const QString &serviceType )
{
  mServiceType = serviceType;
}

QString Messaging::serviceType() const
{
  return mServiceType;
}

void Messaging::setAccountId( const QString &accountId )
{
  mAccountId = accountId;
}

QString Messaging::accountId() const
{
  return mAccountId;
}

void Messaging::setSenderAccountId( const QString &accountId )
{
  mSenderAccountId = accountId;
}

QString Messaging::senderAccountId() const
{
  return mSenderAccountId;
}

bool Messaging::isValid() const
{
  return !mServiceType.isEmpty() && !mAccountId.isEmpty();
}

bool Messaging::operator==( const Messaging &i ) const
{
  return ( i.mServiceType == mServiceType && i.mAccountId == mAccountId &&
           i.mSenderAccountId == mSenderAccountId );
}

bool Messaging::operator!=( const Messaging &i ) const
{
  return !( *this == i );
}

QString Messaging::asString() const
{
  return "(" + mServiceType + ";" + mAccountId + ";" + mSenderAccountId + ")";
}

QDataStream &KABC::operator<<( QDataStream &s, const Messaging &im )
{
  return s << im.mId << im.mServiceType << im.mAccountId << im.mSenderAccountId;
}

QDataStream &KABC::operator>>( QDataStream &s, Messaging &im )
{
  s >> im.mId >> im.mServiceType >> im.mAccountId >> im.mSenderAccountId;

  return s;
}
