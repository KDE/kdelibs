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
#include <kdebug.h>
#include <klocale.h>

#include "address.h"

using namespace KABC;

Address::Address() :
  mEmpty( true ), mType( 0 )
{
  mId = KApplication::randomString( 10 );
}

Address::Address( int type ) :
  mEmpty( true ), mType( type )
{
  mId = KApplication::randomString( 10 );
}

bool Address::operator==( const Address &a ) const
{
  if ( mPostOfficeBox != a.mPostOfficeBox ) return false;
  if ( mExtended != a.mExtended ) return false;
  if ( mStreet != a.mStreet ) return false;
  if ( mLocality != a.mLocality ) return false;
  if ( mRegion != a.mRegion ) return false;
  if ( mPostalCode != a.mPostalCode ) return false;
  if ( mCountry != a.mCountry ) return false;
  if ( mLabel != a.mLabel ) return false;
  
  return true;
}

bool Address::operator!=( const Address &a ) const
{
  return !( a == *this );
}

bool Address::isEmpty() const
{
  if ( mPostOfficeBox.isEmpty() &&
       mExtended.isEmpty() &&
       mStreet.isEmpty() &&
       mLocality.isEmpty() &&
       mRegion.isEmpty() &&
       mPostalCode.isEmpty() &&
       mCountry.isEmpty() &&
       mLabel.isEmpty() ) {
    return true;
  }
  return false;
}

void Address::clear()
{
  *this = Address();
}

void Address::setId( const QString &id )
{
  mEmpty = false;

  mId = id;
}

QString Address::id() const
{
  return mId;
}

void Address::setType( int type )
{
  mEmpty = false;

  mType = type;
}

int Address::type() const
{
  return mType;
}

void Address::setPostOfficeBox( const QString &s )
{
  mEmpty = false;

  mPostOfficeBox = s;
}

QString Address::postOfficeBox() const
{
  return mPostOfficeBox;
}

QString Address::postOfficeBoxLabel()
{
  return i18n("Post Office Box");
}


void Address::setExtended( const QString &s )
{
  mEmpty = false;

  mExtended = s;
}

QString Address::extended() const
{
  return mExtended;
}

QString Address::extendedLabel()
{
  return i18n("Extended Address Information");
}


void Address::setStreet( const QString &s )
{
  mEmpty = false;

  mStreet = s;
}

QString Address::street() const
{
  return mStreet;
}

QString Address::streetLabel()
{
  return i18n("Street");
}


void Address::setLocality( const QString &s )
{
  mEmpty = false;

  mLocality = s;
}

QString Address::locality() const
{
  return mLocality;
}

QString Address::localityLabel()
{
  return i18n("Locality");
}


void Address::setRegion( const QString &s )
{
  mEmpty = false;

  mRegion = s;
}

QString Address::region() const
{
  return mRegion;
}

QString Address::regionLabel()
{
  return i18n("Region");
}


void Address::setPostalCode( const QString &s )
{
  mEmpty = false;

  mPostalCode = s;
}

QString Address::postalCode() const
{
  return mPostalCode;
}

QString Address::postalCodeLabel()
{
  return i18n("Postal Code");
}


void Address::setCountry( const QString &s )
{
  mEmpty = false;

  mCountry = s;
}

QString Address::country() const
{
  return mCountry;
}

QString Address::countryLabel()
{
  return i18n("Country");
}


void Address::setLabel( const QString &s )
{
  mEmpty = false;

  mLabel = s;
}

QString Address::label() const
{
  return mLabel;
}

QString Address::labelLabel()
{
  return i18n("Delivery Label");
}

Address::TypeList Address::typeList()
{
  TypeList list;
  
  list << Dom << Intl << Postal << Parcel << Home << Work << Pref;

  return list;
}

QString Address::typeLabel( int type )
{
  switch ( type ) {
    case Dom:
      return i18n("Domestic");
      break;
    case Intl:
      return i18n("International");
      break;
    case Postal:
      return i18n("Postal");
      break;
    case Parcel:
      return i18n("Parcel");
      break;
    case Home:
      return i18n("Home address");
      break;
    case Work:
      return i18n("Address at work");
      break;
    case Pref:
      return i18n("Preferred address");
      break;
    default:
      return i18n("Unknown");
      break;
  }
}

void Address::dump() const
{
  kdDebug(5700) << "  Address {" << endl;
  kdDebug(5700) << "    Id: " << id() << endl;
  kdDebug(5700) << "    Extended: " << extended() << endl;
  kdDebug(5700) << "    Street: " << street() << endl;
  kdDebug(5700) << "    Postal Code: " << postalCode() << endl;
  kdDebug(5700) << "    Locality: " << locality() << endl;
  kdDebug(5700) << "  }" << endl;
}

QDataStream &KABC::operator<<( QDataStream &s, const Address &addr )
{
    return s << addr.mId << addr.mType << addr.mPostOfficeBox <<
	    addr.mExtended << addr.mStreet << addr.mLocality <<
	    addr.mRegion << addr.mPostalCode << addr.mCountry <<
	    addr.mLabel;
}

QDataStream &KABC::operator>>( QDataStream &s, Address &addr )
{
    s >> addr.mId >> addr.mType >> addr.mPostOfficeBox >> addr.mExtended >>
	    addr.mStreet >> addr.mLocality >> addr.mRegion >>
	    addr.mPostalCode >> addr.mCountry >> addr.mLabel;

    addr.mEmpty = false;

    return s;
}
