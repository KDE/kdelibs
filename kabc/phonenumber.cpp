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

#include <kapplication.h>
#include <klocale.h>

#include "phonenumber.h"

using namespace KABC;

PhoneNumber::PhoneNumber() :
  mType( Home )
{
  init();
}

PhoneNumber::PhoneNumber( const QString &number, int type ) :
  mType( type ), mNumber( number )
{
  init();
}

PhoneNumber::~PhoneNumber()
{
}

void PhoneNumber::init()
{
  mId = KApplication::randomString(8);
}

bool PhoneNumber::operator==( const PhoneNumber &p ) const
{
  if ( mNumber != p.mNumber ) return false;
  if ( mType != p.mType ) return false;
  
  return true;
}

bool PhoneNumber::operator!=( const PhoneNumber &p ) const
{
  return !( p == *this );
}

void PhoneNumber::setId( const QString &id )
{
  mId = id;
}

QString PhoneNumber::id() const
{
  return mId;
}

void PhoneNumber::setNumber( const QString &number )
{
  mNumber = number;
}

QString PhoneNumber::number() const
{
  return mNumber;
}

void PhoneNumber::setType( int type )
{
  mType = type;
}

int PhoneNumber::type() const
{
  return mType;
}

QString PhoneNumber::typeLabel() const
{
  QString label;
  bool first = true;

  TypeList list = typeList();

  TypeList::Iterator it;
  for ( it = list.begin(); it != list.end(); ++it ) {
    if ( ( type() & (*it) ) && ( (*it) != Pref ) ) {
      label.append( ( first ? "" : "/" ) + typeLabel( *it ) );
      if ( first )
        first = false;
    }
  }

  return label;
}

QString PhoneNumber::label() const
{
  return typeLabel( type() );
}

PhoneNumber::TypeList PhoneNumber::typeList()
{
  TypeList list;
  
  list << Home << Work << Msg << Pref << Voice << Fax << Cell << Video
       << Bbs << Modem << Car << Isdn << Pcs << Pager;

  return list;
}

QString PhoneNumber::label( int type )
{
  return typeLabel( type );
}

QString PhoneNumber::typeLabel( int type )
{
  switch ( type ) {
    case Home:
      return i18n("Home phone", "Home");
      break;
    case Work:
      return i18n("Work phone", "Work");
      break;
    case Msg:
      return i18n("Messenger");
      break;  
    case Pref:
      return i18n("Preferred Number");
      break;
    case Voice:
      return i18n("Voice");
      break;  
    case Fax:
      return i18n("Fax");
      break;  
    case Cell:
      return i18n("Mobile Phone", "Mobile" );
      break;  
    case Video:
      return i18n("Video");
      break;
    case Bbs:
      return i18n("Mailbox");
      break;
    case Modem:
      return i18n("Modem");
      break;
    case Car:
      return i18n("Car Phone", "Car" );
      break;
    case Isdn:
      return i18n("ISDN");
      break;
    case Pcs:
      return i18n("PCS");
      break;
    case Pager:
      return i18n("Pager");
      break;
    case Home | Fax:
      return i18n("Home Fax");
      break;
    case Work | Fax:
      return i18n("Work Fax");
      break;
    default:
      return i18n("Other");
  }
}

QDataStream &KABC::operator<<( QDataStream &s, const PhoneNumber &phone )
{
    return s << phone.mId << phone.mType << phone.mNumber;
}

QDataStream &KABC::operator>>( QDataStream &s, PhoneNumber &phone )
{
    s >> phone.mId >> phone.mType >> phone.mNumber;

    return s;
}
