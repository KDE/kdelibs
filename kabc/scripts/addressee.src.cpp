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
#include <ksharedptr.h>
#include <kdebug.h>
#include <kapplication.h>
#include <klocale.h>

#include "resource.h"
#include "addressee.h"

using namespace KABC;

bool matchBinaryPattern( int value, int pattern, int max );

struct Addressee::AddresseeData : public KShared
{
  --VARIABLES--

  PhoneNumber::List phoneNumbers;
  Address::List addresses;
  Key::List keys;
  QStringList emails;
  QStringList categories;
  QStringList custom;

  Resource *resource;

  bool empty;
  bool changed;
};

Addressee::Addressee()
{
  mData = new AddresseeData;
  mData->uid = KApplication::randomString( 10 );
  mData->empty = true;
  mData->changed = false;
  mData->resource = 0;
}

Addressee::~Addressee()
{
}

Addressee::Addressee( const Addressee &a )
{
  mData = a.mData;
}

Addressee &Addressee::operator=( const Addressee &a )
{
  mData = a.mData;
  return (*this);
}

Addressee Addressee::copy()
{
  Addressee a;
  *(a.mData) = *mData;
  return a;
}

void Addressee::detach()
{
  if ( mData.count() == 1 ) return;
  *this = copy();
}

bool Addressee::operator==( const Addressee &a ) const
{
  --EQUALSTEST--
  if ( ( mData->url.isValid() || a.mData->url.isValid() ) &&
       ( mData->url != a.mData->url ) ) return false;
  if ( mData->phoneNumbers != a.mData->phoneNumbers ) return false;
  if ( mData->addresses != a.mData->addresses ) return false;
  if ( mData->keys != a.mData->keys ) return false;
  if ( mData->emails != a.mData->emails ) return false;
  if ( mData->categories != a.mData->categories ) return false;
  if ( mData->custom != a.mData->custom ) return false;

  return true;
}

bool Addressee::operator!=( const Addressee &a ) const
{
  return !( a == *this );
}

bool Addressee::isEmpty() const
{
  return mData->empty;
}

--DEFINITIONS--

void Addressee::setNameFromString( const QString &str )
{
  setFormattedName( str );
  setName( str );

  QStringList titles;
  titles += i18n( "Dr." );
  titles += i18n( "Miss" );
  titles += i18n( "Mr." );
  titles += i18n( "Mrs." );
  titles += i18n( "Ms." );
  titles += i18n( "Prof." );

  QStringList suffixes;
  suffixes += i18n( "I" );
  suffixes += i18n( "II" );
  suffixes += i18n( "III" );
  suffixes += i18n( "Jr." );
  suffixes += i18n( "Sr." );

  QStringList prefixes;
  prefixes += "van";
  prefixes += "von";
  prefixes += "de";

  // clear all name parts
  setPrefix( "" );
  setGivenName( "" );
  setAdditionalName( "" );
  setFamilyName( "" );
  setSuffix( "" );

  if ( str.isEmpty() )
    return;

  int i = str.find(',');
  if( i < 0 ) {
    QStringList parts = QStringList::split( " ", str );
    int leftOffset = 0;
    int rightOffset = parts.count() - 1;

    QString suffix;
    while ( rightOffset >= 0 ) {
      if ( suffixes.contains( parts[ rightOffset ] ) ) {
        suffix.prepend(parts[ rightOffset ] + (suffix.isEmpty() ? "" : " "));
        rightOffset--;
      } else
        break;
    }
    setSuffix( suffix );

    if ( rightOffset - 1 >= 0 && prefixes.contains( parts[ rightOffset - 1 ].lower() ) ) {
      setFamilyName( parts[ rightOffset - 1 ] + " " + parts[ rightOffset ] );
      rightOffset--;
    } else
      setFamilyName( parts[ rightOffset ] );

    QString prefix;
    while ( leftOffset < rightOffset ) {
      if ( titles.contains( parts[ leftOffset ] ) ) {
        prefix.append( ( prefix.isEmpty() ? "" : " ") + parts[ leftOffset ] );
        leftOffset++;
      } else
        break;
    }
    setPrefix( prefix );

    if ( leftOffset < rightOffset ) {
      setGivenName( parts[ leftOffset ] );
      leftOffset++;
    }

    QString additionalName;
    while ( leftOffset < rightOffset ) {
      additionalName.append( ( additionalName.isEmpty() ? "" : " ") + parts[ leftOffset ] );
      leftOffset++;
    }
    setAdditionalName( additionalName );
  } else {
    QString part1 = str.left( i );
    QString part2 = str.mid( i + 1 );

    QStringList parts = QStringList::split( " ", part1 );
    int leftOffset = 0;
    int rightOffset = parts.count() - 1;

    QString suffix;
    while ( rightOffset >= 0 ) {
      if ( suffixes.contains( parts[ rightOffset ] ) ) {
        suffix.prepend(parts[ rightOffset ] + (suffix.isEmpty() ? "" : " "));
        rightOffset--;
      } else
        break;
    }
    setSuffix( suffix );

    if ( rightOffset - 1 >= 0 && prefixes.contains( parts[ rightOffset - 1 ].lower() ) ) {
      setFamilyName( parts[ rightOffset - 1 ] + " " + parts[ rightOffset ] );
      rightOffset--;
    } else
      setFamilyName( parts[ rightOffset ] );

    QString prefix;
    while ( leftOffset < rightOffset ) {
      if ( titles.contains( parts[ leftOffset ] ) ) {
        prefix.append( ( prefix.isEmpty() ? "" : " ") + parts[ leftOffset ] );
        leftOffset++;
      } else
        break;
    }

    parts = QStringList::split( " ", part2 );

    leftOffset = 0;
    rightOffset = parts.count();

    while ( leftOffset < rightOffset ) {
      if ( titles.contains( parts[ leftOffset ] ) ) {
        prefix.append( ( prefix.isEmpty() ? "" : " ") + parts[ leftOffset ] );
        leftOffset++;
      } else
        break;
    }
    setPrefix( prefix );

    if ( leftOffset < rightOffset ) {
      setGivenName( parts[ leftOffset ] );
      leftOffset++;
    }

    QString additionalName;
    while ( leftOffset < rightOffset ) {
      additionalName.append( ( additionalName.isEmpty() ? "" : " ") + parts[ leftOffset ] );
      leftOffset++;
    }
    setAdditionalName( additionalName );
  }
}

QString Addressee::realName() const
{
  QString n = prefix() + " " + givenName() + " " + additionalName() + " " +
              familyName() + " " + suffix();
  n = n.simplifyWhiteSpace();             

  if ( n.isEmpty() ) n = name();
  
  return n;
}

QString Addressee::fullEmail( const QString &email ) const
{
  QString e;
  if ( email.isNull() ) {
    e = preferredEmail();
  } else {
    e = email;
  }
  if ( e.isEmpty() ) return QString::null;
  
  QString text;
  if ( !realName().isEmpty() ) text = realName() + " ";
  text.append( "<" + e + ">" );

  return text;
}

void Addressee::insertEmail( const QString &email, bool preferred )
{
  detach();

  QStringList::Iterator it = mData->emails.find( email );

  if ( it != mData->emails.end() ) {
    if ( !preferred || it == mData->emails.begin() ) return;
    mData->emails.remove( it );
    mData->emails.prepend( email );
  } else {
    if ( preferred ) {
      mData->emails.prepend( email );
    } else {
      mData->emails.append( email );
    }
  }
}

void Addressee::removeEmail( const QString &email )
{
  detach();

  QStringList::Iterator it = mData->emails.find( email );
  if ( it == mData->emails.end() ) return;

  mData->emails.remove( it );
}

QString Addressee::preferredEmail() const
{
  if ( mData->emails.count() == 0 ) return QString::null;
  else return mData->emails.first();
}

QStringList Addressee::emails() const
{
  return mData->emails;
}

void Addressee::insertPhoneNumber( const PhoneNumber &phoneNumber )
{
  detach();
  mData->empty = false;

  PhoneNumber::List::Iterator it;
  for( it = mData->phoneNumbers.begin(); it != mData->phoneNumbers.end(); ++it ) {
    if ( (*it).id() == phoneNumber.id() ) {
      *it = phoneNumber;
      return;
    }
  }
  mData->phoneNumbers.append( phoneNumber );
}

void Addressee::removePhoneNumber( const PhoneNumber &phoneNumber )
{
  detach();

  PhoneNumber::List::Iterator it;
  for( it = mData->phoneNumbers.begin(); it != mData->phoneNumbers.end(); ++it ) {
    if ( (*it).id() == phoneNumber.id() ) {
      mData->phoneNumbers.remove( it );
      return;
    }
  }
}

PhoneNumber Addressee::phoneNumber( int type ) const
{
  PhoneNumber phoneNumber( "", type );
  PhoneNumber::List::ConstIterator it;
  for( it = mData->phoneNumbers.begin(); it != mData->phoneNumbers.end(); ++it ) {
    if ( matchBinaryPattern( (*it).type(), type, PhoneNumber::Pager ) ) {
      if ( (*it).type() & PhoneNumber::Pref )
        return (*it);
      else if ( phoneNumber.number().isEmpty() )
        phoneNumber = (*it);
    }
  }

  return phoneNumber;
}

PhoneNumber::List Addressee::phoneNumbers() const
{
  return mData->phoneNumbers;
}

PhoneNumber::List Addressee::phoneNumbers( int type ) const
{
  PhoneNumber::List list;

  PhoneNumber::List::ConstIterator it;
  for( it = mData->phoneNumbers.begin(); it != mData->phoneNumbers.end(); ++it ) {
    if ( matchBinaryPattern( (*it).type(), type, PhoneNumber::Pager ) ) {
      list.append( *it );
    }
  }
  return list;
}

PhoneNumber Addressee::findPhoneNumber( const QString &id ) const
{
  PhoneNumber::List::ConstIterator it;
  for( it = mData->phoneNumbers.begin(); it != mData->phoneNumbers.end(); ++it ) {
    if ( (*it).id() == id ) {
      return *it;
    }
  }
  return PhoneNumber();
}

void Addressee::insertKey( const Key &key )
{
  detach();
  mData->empty = false;

  Key::List::Iterator it;
  for( it = mData->keys.begin(); it != mData->keys.end(); ++it ) {
    if ( (*it).id() == key.id() ) {
      *it = key;
      return;
    }
  }
  mData->keys.append( key );
}

void Addressee::removeKey( const Key &key )
{
  detach();

  Key::List::Iterator it;
  for( it = mData->keys.begin(); it != mData->keys.end(); ++it ) {
    if ( (*it).id() == key.id() ) {
      mData->keys.remove( key );
      return;
    }
  }
}

Key Addressee::key( int type, QString customTypeString ) const
{
  Key::List::ConstIterator it;
  for( it = mData->keys.begin(); it != mData->keys.end(); ++it ) {
    if ( (*it).type() == type ) {
      if ( type == Key::Custom ) {
        if ( customTypeString.isEmpty() ) {
          return *it;
        } else {
          if ( (*it).customTypeString() == customTypeString )
            return (*it);
        }
      } else {
        return *it;
      }
    }
  }
  return Key( QString(), type );
}

Key::List Addressee::keys() const
{
  return mData->keys;
}

Key::List Addressee::keys( int type, QString customTypeString ) const
{
  Key::List list;

  Key::List::ConstIterator it;
  for( it = mData->keys.begin(); it != mData->keys.end(); ++it ) {
    if ( (*it).type() == type ) {
      if ( type == Key::Custom ) {
        if ( customTypeString.isEmpty() ) {
          list.append(*it);
        } else {
          if ( (*it).customTypeString() == customTypeString )
            list.append(*it);
        }
      } else {
        list.append(*it);
      }
    }
  }
  return list;
}

Key Addressee::findKey( const QString &id ) const
{
  Key::List::ConstIterator it;
  for( it = mData->keys.begin(); it != mData->keys.end(); ++it ) {
    if ( (*it).id() == id ) {
      return *it;
    }
  }
  return Key();
}

QString Addressee::asString() const
{
  return "Smith, special agent Smith...";
}

void Addressee::dump() const
{
  kdDebug(5700) << "Addressee {" << endl;

  --DEBUG--
  
  kdDebug(5700) << "  Emails {" << endl;
  QStringList e = emails();
  QStringList::ConstIterator it;
  for( it = e.begin(); it != e.end(); ++it ) {
    kdDebug(5700) << "    " << (*it) << endl;
  }
  kdDebug(5700) << "  }" << endl;

  kdDebug(5700) << "  PhoneNumbers {" << endl;
  PhoneNumber::List p = phoneNumbers();
  PhoneNumber::List::ConstIterator it2;
  for( it2 = p.begin(); it2 != p.end(); ++it2 ) {
    kdDebug(5700) << "    Type: " << int((*it2).type()) << " Number: " << (*it2).number() << endl;
  }
  kdDebug(5700) << "  }" << endl;

  Address::List a = addresses();
  Address::List::ConstIterator it3;
  for( it3 = a.begin(); it3 != a.end(); ++it3 ) {
    (*it3).dump();
  }

  kdDebug(5700) << "  Keys {" << endl;
  Key::List k = keys();
  Key::List::ConstIterator it4;
  for( it4 = k.begin(); it4 != k.end(); ++it4 ) {
    kdDebug(5700) << "    Type: " << int((*it4).type()) <<
                     " Key: " << (*it4).textData() <<
                     " CustomString: " << (*it4).customTypeString() << endl;
  }
  kdDebug(5700) << "  }" << endl;

  kdDebug(5700) << "}" << endl;
}


void Addressee::insertAddress( const Address &address )
{
  detach();
  mData->empty = false;

  Address::List::Iterator it;
  for( it = mData->addresses.begin(); it != mData->addresses.end(); ++it ) {
    if ( (*it).id() == address.id() ) {
      *it = address;
      return;
    }
  }
  mData->addresses.append( address );
}

void Addressee::removeAddress( const Address &address )
{
  detach();

  Address::List::Iterator it;
  for( it = mData->addresses.begin(); it != mData->addresses.end(); ++it ) {
    if ( (*it).id() == address.id() ) {
      mData->addresses.remove( it );
      return;
    }
  }
}

Address Addressee::address( int type ) const
{
  Address address( type );
  Address::List::ConstIterator it;
  for( it = mData->addresses.begin(); it != mData->addresses.end(); ++it ) {
    if ( matchBinaryPattern( (*it).type(), type, Address::Pref ) ) {
      if ( (*it).type() & Address::Pref )
        return (*it);
      else if ( address.isEmpty() )
        address = (*it);
    }
  }

  return address;
}

Address::List Addressee::addresses() const
{
  return mData->addresses;
}

Address::List Addressee::addresses( int type ) const
{
  Address::List list;

  Address::List::ConstIterator it;
  for( it = mData->addresses.begin(); it != mData->addresses.end(); ++it ) {
    if ( matchBinaryPattern( (*it).type(), type , Address::Pref ) ) {
      list.append( *it );
    }
  }

  return list;
}

Address Addressee::findAddress( const QString &id ) const
{
  Address::List::ConstIterator it;
  for( it = mData->addresses.begin(); it != mData->addresses.end(); ++it ) {
    if ( (*it).id() == id ) {
      return *it;
    }
  }
  return Address();
}

void Addressee::insertCategory( const QString &c )
{
  detach();
  mData->empty = false;

  if ( mData->categories.contains( c ) ) return;

  mData->categories.append( c );
}

void Addressee::removeCategory( const QString &c )
{
  detach();

  QStringList::Iterator it = mData->categories.find( c );
  if ( it == mData->categories.end() ) return;

  mData->categories.remove( it );
}

bool Addressee::hasCategory( const QString &c ) const
{
  return ( mData->categories.contains( c ) );
}

void Addressee::setCategories( const QStringList &c )
{
  detach();
  mData->empty = false;

  mData->categories = c;
}

QStringList Addressee::categories() const
{
  return mData->categories;
}

void Addressee::insertCustom( const QString &app, const QString &name,
                              const QString &value )
{
  if ( value.isNull() || name.isEmpty() || app.isEmpty() ) return;

  detach();
  mData->empty = false;
  
  QString qualifiedName = app + "-" + name + ":";
  
  QStringList::Iterator it;
  for( it = mData->custom.begin(); it != mData->custom.end(); ++it ) {
    if ( (*it).startsWith( qualifiedName ) ) {
      (*it) = qualifiedName + value;
      return;
    }
  }
  
  mData->custom.append( qualifiedName + value );
}

void Addressee::removeCustom( const QString &app, const QString &name)
{
  detach();
  
  QString qualifiedName = app + "-" + name + ":";
  
  QStringList::Iterator it;
  for( it = mData->custom.begin(); it != mData->custom.end(); ++it ) {
    if ( (*it).startsWith( qualifiedName ) ) {
      mData->custom.remove( it );
      return;
    }
  }
}

QString Addressee::custom( const QString &app, const QString &name ) const
{
  QString qualifiedName = app + "-" + name + ":";
  QString value;
  
  QStringList::ConstIterator it;
  for( it = mData->custom.begin(); it != mData->custom.end(); ++it ) {
    if ( (*it).startsWith( qualifiedName ) ) {
      value = (*it).mid( (*it).find( ":" ) + 1 );
      break;
    }
  }
  
  return value;
}

void Addressee::setCustoms( const QStringList &l )
{
  detach();
  mData->empty = false;
  
  mData->custom = l;
}

QStringList Addressee::customs() const
{
  return mData->custom;
}

void Addressee::parseEmailAddress( const QString &rawEmail, QString &fullName, 
                                   QString &email)
{
  int startPos, endPos, len;
  QString partA, partB, result;
  char endCh = '>';
  
  startPos = rawEmail.find('<');
  if (startPos < 0)
  {
    startPos = rawEmail.find('(');
    endCh = ')';
  }
  if (startPos < 0)
  {
    // We couldn't find any separators, so we assume the whole string
    // is the email address
    email = rawEmail;
    fullName = "";
  }
  else 
  {
    // We have a start position, try to find an end
    endPos = rawEmail.find(endCh, startPos+1);
    
    if (endPos < 0) 
    {
      // We couldn't find the end of the email address. We can only
      // assume the entire string is the email address.
      email = rawEmail;
      fullName = "";
    }
    else
    {
      // We have a start and end to the email address
      
      // Grab the name part
      fullName = rawEmail.left(startPos).stripWhiteSpace();
      
      // grab the email part
      email = rawEmail.mid(startPos+1, endPos-startPos-1).stripWhiteSpace();

      // Check that we do not have any extra characters on the end of the
      // strings
      len = fullName.length();
      if (fullName[0]=='"' && fullName[len-1]=='"')
        fullName = fullName.mid(1, len-2);
      else if (fullName[0]=='<' && fullName[len-1]=='>')
        fullName = fullName.mid(1, len-2);
      else if (fullName[0]=='(' && fullName[len-1]==')')
        fullName = fullName.mid(1, len-2);
    }
  }
}

void Addressee::setResource( Resource *resource )
{
    detach();
    mData->resource = resource;
} 

Resource *Addressee::resource() const
{
    return mData->resource;
}

void Addressee::setChanged( bool value )
{
    detach();
    mData->changed = value;
}

bool Addressee::changed() const
{
    return mData->changed;
}

QDataStream &KABC::operator<<( QDataStream &s, const Addressee &a )
{
  if (!a.mData) return s;

  --STREAMOUT--
  s << a.mData->phoneNumbers;
  s << a.mData->addresses;
  s << a.mData->emails;
  s << a.mData->categories;
  s << a.mData->custom;
  s << a.mData->keys;
  return s;
}

QDataStream &KABC::operator>>( QDataStream &s, Addressee &a )
{
  if (!a.mData) return s;

  --STREAMIN--
  s >> a.mData->phoneNumbers;
  s >> a.mData->addresses;
  s >> a.mData->emails;
  s >> a.mData->categories;
  s >> a.mData->custom;
  s >> a.mData->keys;

  a.mData->empty = false;

  return s;
}

bool matchBinaryPattern( int value, int pattern, int max )
{
  if ( pattern == 0 ) {
    if ( value != 0 )
      return false;
    else
      return true;
  }

  int counter = 0;
  while ( 1 ) {
    if ( ( pattern & ( 1 << counter ) ) && !( value & ( 1 << counter ) ) )
      return false;

    if ( ( 1 << counter ) == max )
      break;

    counter++;
  }

  return true;
}
