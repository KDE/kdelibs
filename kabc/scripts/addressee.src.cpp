/*
    This file is part of libkabc.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 Carsten Pfeiffer <pfeiffer@kde.org>
    Copyright (c) 2005 Ingo Kloecker <kloecker@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QList>
#include <QRegExp>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <krandom.h>
#include <ksharedptr.h>

#include "addresseehelper.h"
#include "field.h"
#include "resource.h"
#include "sortmode.h"

#include "addressee.h"

using namespace KABC;

static bool matchBinaryPattern( int value, int pattern );

template <class L>
static bool listEquals( const QList<L>&, const QList<L>& );
static bool listEquals( const QStringList&, const QStringList& );
static bool emailsEquals( const QStringList&, const QStringList& );

KABC::SortMode *Addressee::mSortMode = 0;

struct Addressee::AddresseeData : public KShared
{
  QString uid;
  --VARIABLES--

  PhoneNumber::List phoneNumbers;
  Address::List addresses;
  Key::List keys;
  QStringList emails;
  QStringList categories;
  QStringList custom;

  Resource *resource;

  bool empty    :1;
  bool changed  :1;
};

// #### missing: KStaticDeleter
Addressee::AddresseeData* Addressee::shared_null = 0;

Addressee::AddresseeData* Addressee::makeSharedNull()
{
  Addressee::shared_null = new AddresseeData;
  shared_null->ref.ref(); // make sure it's not deleted
  shared_null->empty = true;
  shared_null->changed = false;
  shared_null->resource = 0;
  return shared_null;
}

Addressee::Addressee()
{
  mData = shared_null ? shared_null : makeSharedNull();
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

void Addressee::detach()
{
  if ( mData.data() == shared_null ) {
    mData = new AddresseeData;
    mData->empty = true;
    mData->changed = false;
    mData->resource = 0;
    mData->uid = KRandom::randomString( 10 );
    return;
  }
  else if (mData.isUnique()) return;

  AddresseeData data = *mData;
  mData = new AddresseeData( data );
}

bool Addressee::operator==( const Addressee &a ) const
{
  if ( uid() != a.uid() ) {
    kdDebug(5700) << "uid differs" << endl;
    return false;
  }
  --EQUALSTEST--
  if ( ( mData->url.isValid() || a.mData->url.isValid() ) &&
       ( mData->url != a.mData->url ) ) {
    kdDebug(5700) << "url differs" << endl;
    return false;
  }
  if ( !listEquals( mData->phoneNumbers, a.mData->phoneNumbers ) ) {
    kdDebug(5700) << "phoneNumbers differs" << endl;
    return false;
  }
  if ( !listEquals( mData->addresses, a.mData->addresses ) ) {
    kdDebug(5700) << "addresses differs" << endl;
    return false;
  }
  if ( !listEquals( mData->keys, a.mData->keys ) ) {
    kdDebug(5700) << "keys differs" << endl;
    return false;
  }
  if ( !emailsEquals( mData->emails, a.mData->emails ) ) {
    kdDebug(5700) << "emails differs" << endl;
    return false;
  }
  if ( !listEquals( mData->categories, a.mData->categories ) ) {
    kdDebug(5700) << "categories differs" << endl;
    return false;
  }
  if ( !listEquals( mData->custom, a.mData->custom ) ) {
    kdDebug(5700) << "custom differs" << endl;
    return false;
  }

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

void Addressee::setUid( const QString &id )
{
  if ( id == mData->uid ) return;
  detach();
  mData->empty = false;
  mData->uid = id;
}

QString Addressee::uid() const
{
  return mData->uid;
}

QString Addressee::uidLabel()
{
  return i18n("Unique Identifier");
}

--DEFINITIONS--

void Addressee::setNameFromString( const QString &str )
{
  setFormattedName( str );
  setName( str );

  // clear all name parts
  setPrefix( QString() );
  setGivenName( QString() );
  setAdditionalName( QString() );
  setFamilyName( QString() );
  setSuffix( QString() );

  if ( str.isEmpty() )
    return;

  static QString spaceStr = " ";
  static QString emptyStr = "";
  AddresseeHelper *helper = AddresseeHelper::self();

  int i = str.indexOf( ',' );
  if( i < 0 ) {
    QStringList parts = str.split( spaceStr );
    int leftOffset = 0;
    int rightOffset = parts.count() - 1;

    QString suffix;
    while ( rightOffset >= 0 ) {
      if ( helper->containsSuffix( parts[ rightOffset ] ) ) {
        suffix.prepend(parts[ rightOffset ] + (suffix.isEmpty() ? emptyStr : spaceStr));
        rightOffset--;
      } else
        break;
    }
    setSuffix( suffix );

    if ( rightOffset < 0 )
      return;

    if ( rightOffset - 1 >= 0 && helper->containsPrefix( parts[ rightOffset - 1 ].toLower() ) ) {
      setFamilyName( parts[ rightOffset - 1 ] + spaceStr + parts[ rightOffset ] );
      rightOffset--;
    } else {
      if ( helper->tradeAsFamilyName() )
        setFamilyName( parts[ rightOffset ] );
      else
        setGivenName( parts[ rightOffset ] );
    }

    QString prefix;
    while ( leftOffset < rightOffset ) {
      if ( helper->containsTitle( parts[ leftOffset ] ) ) {
        prefix.append( (prefix.isEmpty() ? emptyStr : spaceStr) + parts[ leftOffset ] );
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
      additionalName.append( (additionalName.isEmpty() ? emptyStr : spaceStr) + parts[ leftOffset ] );
      leftOffset++;
    }
    setAdditionalName( additionalName );
  } else {
    QString part1 = str.left( i );
    QString part2 = str.mid( i + 1 );

    QStringList parts = part1.split( spaceStr );
    int leftOffset = 0;
    int rightOffset = parts.count() - 1;

    if ( parts.count() > 0 ) {

      QString suffix;
      while ( rightOffset >= 0 ) {
        if ( helper->containsSuffix( parts[ rightOffset ] ) ) {
          suffix.prepend( parts[ rightOffset ] + (suffix.isEmpty() ? emptyStr : spaceStr) );
          rightOffset--;
        } else
          break;
      }
      setSuffix( suffix );

      if ( rightOffset - 1 >= 0 && helper->containsPrefix( parts[ rightOffset - 1 ].toLower() ) ) {
        setFamilyName( parts[ rightOffset - 1 ] + spaceStr + parts[ rightOffset ] );
        rightOffset--;
      } else
        setFamilyName( parts[ rightOffset ] );

      QString prefix;
      while ( leftOffset < rightOffset ) {
        if ( helper->containsTitle( parts[ leftOffset ] ) ) {
          prefix.append( (prefix.isEmpty() ? emptyStr : spaceStr) + parts[ leftOffset ] );
          leftOffset++;
        } else
          break;
      }
    } else {
      setPrefix( "" );
      setFamilyName( "" );
      setSuffix( "" );
    }

    parts = part2.split( spaceStr );

    leftOffset = 0;
    rightOffset = parts.count();

    if ( parts.count() > 0 ) {

      QString prefix;
      while ( leftOffset < rightOffset ) {
        if ( helper->containsTitle( parts[ leftOffset ] ) ) {
          prefix.append( (prefix.isEmpty() ? emptyStr : spaceStr) + parts[ leftOffset ] );
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
        additionalName.append( (additionalName.isEmpty() ? emptyStr : spaceStr) + parts[ leftOffset ] );
        leftOffset++;
      }
      setAdditionalName( additionalName );
    } else {
      setGivenName( "" );
      setAdditionalName( "" );
    }
  }
}

QString Addressee::realName() const
{
  QString n( formattedName() );
  if ( !n.isEmpty() )
	return n;

  n = assembledName();
  if ( !n.isEmpty() )
	return n;

  n = name();
  if ( !n.isEmpty() )
	return n;

  return organization();
}

QString Addressee::assembledName() const
{
  QString name = prefix() + " " + givenName() + " " + additionalName() + " " +
              familyName() + " " + suffix();

  return name.simplified();
}

QString Addressee::fullEmail( const QString &email ) const
{
  QString e;
  if ( email.isNull() ) {
    e = preferredEmail();
  } else {
    e = email;
  }
  if ( e.isEmpty() ) return QString();

  QString text;
  if ( realName().isEmpty() )
    text = e;
  else {
    QRegExp needQuotes( "[^ 0-9A-Za-z\\x0080-\\xFFFF]" );
    if ( realName().indexOf( needQuotes ) != -1 )
      text = "\"" + realName() + "\" <" + e + ">";
    else
      text = realName() + " <" + e + ">";
  }

  return text;
}

void Addressee::insertEmail( const QString &email, bool preferred )
{
  if ( email.simplified().isEmpty() )
    return;

  detach();

  if ( !(mData->emails.contains( email )) ) {
    if ( !preferred || mData->emails.first() == email )
      return;

    mData->emails.removeAll( email );
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
  if ( mData->emails.contains( email ) ) {
    detach();
    mData->emails.removeAll( email );
  }
}

QString Addressee::preferredEmail() const
{
  if ( mData->emails.count() == 0 )
    return QString();
  else
    return mData->emails.first();
}

QStringList Addressee::emails() const
{
  return mData->emails;
}

void Addressee::setEmails( const QStringList& emails )
{
  detach();

  mData->emails = emails;
}
void Addressee::insertPhoneNumber( const PhoneNumber &phoneNumber )
{
  detach();
  mData->empty = false;

  PhoneNumber::List::Iterator it;
  for ( it = mData->phoneNumbers.begin(); it != mData->phoneNumbers.end(); ++it ) {
    if ( (*it).id() == phoneNumber.id() ) {
      *it = phoneNumber;
      return;
    }
  }
  if ( !phoneNumber.number().simplified().isEmpty() )
    mData->phoneNumbers.append( phoneNumber );
}

void Addressee::removePhoneNumber( const PhoneNumber &phoneNumber )
{
  PhoneNumber::List::Iterator it;
  for ( it = mData->phoneNumbers.begin(); it != mData->phoneNumbers.end(); ++it ) {
    if ( (*it).id() == phoneNumber.id() ) {
      detach();
      mData->phoneNumbers.erase( it );
      return;
    }
  }
}

PhoneNumber Addressee::phoneNumber( int type ) const
{
  PhoneNumber phoneNumber( "", type );
  PhoneNumber::List::ConstIterator it;
  for ( it = mData->phoneNumbers.constBegin(); it != mData->phoneNumbers.constEnd(); ++it ) {
    if ( matchBinaryPattern( (*it).type(), type ) ) {
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
  for ( it = mData->phoneNumbers.constBegin(); it != mData->phoneNumbers.constEnd(); ++it ) {
    if ( matchBinaryPattern( (*it).type(), type ) ) {
      list.append( *it );
    }
  }
  return list;
}

PhoneNumber Addressee::findPhoneNumber( const QString &id ) const
{
  PhoneNumber::List::ConstIterator it;
  for ( it = mData->phoneNumbers.constBegin(); it != mData->phoneNumbers.constEnd(); ++it ) {
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
  for ( it = mData->keys.begin(); it != mData->keys.end(); ++it ) {
    if ( (*it).id() == key.id() ) {
      *it = key;
      return;
    }
  }
  mData->keys.append( key );
}

void Addressee::removeKey( const Key &key )
{
  Key::List::Iterator it;
  for ( it = mData->keys.begin(); it != mData->keys.end(); ++it ) {
    if ( (*it).id() == key.id() ) {
      detach();
      mData->keys.removeAll( key );
      return;
    }
  }
}

Key Addressee::key( int type, QString customTypeString ) const
{
  Key::List::ConstIterator it;
  for ( it = mData->keys.constBegin(); it != mData->keys.constEnd(); ++it ) {
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

void Addressee::setKeys( const Key::List& list )
{
  detach();
  mData->keys = list;
}

Key::List Addressee::keys() const
{
  return mData->keys;
}

Key::List Addressee::keys( int type, QString customTypeString ) const
{
  Key::List list;

  Key::List::ConstIterator it;
  for ( it = mData->keys.constBegin(); it != mData->keys.constEnd(); ++it ) {
    if ( (*it).type() == type ) {
      if ( type == Key::Custom ) {
        if ( customTypeString.isEmpty() ) {
          list.append( *it );
        } else {
          if ( (*it).customTypeString() == customTypeString )
            list.append( *it );
        }
      } else {
        list.append( *it );
      }
    }
  }
  return list;
}

Key Addressee::findKey( const QString &id ) const
{
  Key::List::ConstIterator it;
  for ( it = mData->keys.constBegin(); it != mData->keys.constEnd(); ++it ) {
    if ( (*it).id() == id ) {
      return *it;
    }
  }
  return Key();
}

QString Addressee::asString() const
{
  return "Smith, agent Smith...";
}

void Addressee::dump() const
{
  kdDebug(5700) << "Addressee {" << endl;

  kdDebug(5700) << "  Uid: '" << uid() << "'" << endl;

  --DEBUG--

  kdDebug(5700) << "  Emails {" << endl;
  const QStringList e = emails();
  QStringList::ConstIterator it;
  for ( it = e.begin(); it != e.end(); ++it ) {
    kdDebug(5700) << "    " << (*it) << endl;
  }
  kdDebug(5700) << "  }" << endl;

  kdDebug(5700) << "  PhoneNumbers {" << endl;
  const PhoneNumber::List p = phoneNumbers();
  PhoneNumber::List::ConstIterator it2;
  for ( it2 = p.begin(); it2 != p.end(); ++it2 ) {
    kdDebug(5700) << "    Type: " << int((*it2).type()) << " Number: " << (*it2).number() << endl;
  }
  kdDebug(5700) << "  }" << endl;

  const Address::List a = addresses();
  Address::List::ConstIterator it3;
  for ( it3 = a.begin(); it3 != a.end(); ++it3 ) {
    (*it3).dump();
  }

  kdDebug(5700) << "  Keys {" << endl;
  const Key::List k = keys();
  Key::List::ConstIterator it4;
  for ( it4 = k.begin(); it4 != k.end(); ++it4 ) {
    kdDebug(5700) << "    Type: " << int((*it4).type()) <<
                     " Key: " << (*it4).textData() <<
                     " CustomString: " << (*it4).customTypeString() << endl;
  }
  kdDebug(5700) << "  }" << endl;

  kdDebug(5700) << "}" << endl;
}


void Addressee::insertAddress( const Address &address )
{
  if ( address.isEmpty() )
    return;

  detach();
  mData->empty = false;

  Address::List::Iterator it;
  for ( it = mData->addresses.begin(); it != mData->addresses.end(); ++it ) {
    if ( (*it).id() == address.id() ) {
      *it = address;
      return;
    }
  }

  mData->addresses.append( address );
}

void Addressee::removeAddress( const Address &address )
{
  Address::List::Iterator it;
  for ( it = mData->addresses.begin(); it != mData->addresses.end(); ++it ) {
    if ( (*it).id() == address.id() ) {
      detach();
      mData->addresses.erase( it );
      return;
    }
  }
}

Address Addressee::address( int type ) const
{
  Address address( type );
  Address::List::ConstIterator it;
  for ( it = mData->addresses.constBegin(); it != mData->addresses.constEnd(); ++it ) {
    if ( matchBinaryPattern( (*it).type(), type ) ) {
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
  for ( it = mData->addresses.constBegin(); it != mData->addresses.constEnd(); ++it ) {
    if ( matchBinaryPattern( (*it).type(), type ) ) {
      list.append( *it );
    }
  }

  return list;
}

Address Addressee::findAddress( const QString &id ) const
{
  Address::List::ConstIterator it;
  for ( it = mData->addresses.constBegin(); it != mData->addresses.constEnd(); ++it ) {
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

  if ( mData->categories.contains( c ) )
    return;

  mData->categories.append( c );
}

void Addressee::removeCategory( const QString &category )
{
  if ( mData->categories.contains( category ) ) {
    detach();
    mData->categories.removeAll( category );
  }
}

bool Addressee::hasCategory( const QString &category ) const
{
  return mData->categories.contains( category );
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
  if ( value.isEmpty() || name.isEmpty() || app.isEmpty() ) return;

  detach();
  mData->empty = false;

  QString qualifiedName = app + "-" + name + ":";

  QStringList::Iterator it;
  for ( it = mData->custom.begin(); it != mData->custom.end(); ++it ) {
    if ( (*it).startsWith( qualifiedName ) ) {
      (*it) = qualifiedName + value;
      return;
    }
  }

  mData->custom.append( qualifiedName + value );
}

void Addressee::removeCustom( const QString &app, const QString &name )
{
  const QString qualifiedName = app + "-" + name + ":";

  QStringList::Iterator it;
  for ( it = mData->custom.begin(); it != mData->custom.end(); ++it ) {
    if ( (*it).startsWith( qualifiedName ) ) {
      detach();
      mData->custom.erase( it );
      return;
    }
  }
}

QString Addressee::custom( const QString &app, const QString &name ) const
{
  QString qualifiedName = app + "-" + name + ":";
  QString value;

  QStringList::ConstIterator it;
  for ( it = mData->custom.constBegin(); it != mData->custom.constEnd(); ++it ) {
    if ( (*it).startsWith( qualifiedName ) ) {
      value = (*it).mid( (*it).indexOf( ":" ) + 1 );
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
                                   QString &email )
{
  // This is a simplified version of KPIM::splitAddress().

  fullName = "";
  email = "";
  if ( rawEmail.isEmpty() )
    return; // KPIM::AddressEmpty;

  // The code works on 8-bit strings, so convert the input to UTF-8.
  QByteArray address = rawEmail.toUtf8();

  QByteArray displayName;
  QByteArray addrSpec;
  QByteArray comment;

  // The following is a primitive parser for a mailbox-list (cf. RFC 2822).
  // The purpose is to extract a displayable string from the mailboxes.
  // Comments in the addr-spec are not handled. No error checking is done.

  enum { TopLevel, InComment, InAngleAddress } context = TopLevel;
  bool inQuotedString = false;
  int commentLevel = 0;
  bool stop = false;

  for ( char* p = address.data(); *p && !stop; ++p ) {
    switch ( context ) {
    case TopLevel : {
      switch ( *p ) {
      case '"' : inQuotedString = !inQuotedString;
                 displayName += *p;
                 break;
      case '(' : if ( !inQuotedString ) {
                   context = InComment;
                   commentLevel = 1;
                 }
                 else
                   displayName += *p;
                 break;
      case '<' : if ( !inQuotedString ) {
                   context = InAngleAddress;
                 }
                 else
                   displayName += *p;
                 break;
      case '\\' : // quoted character
                 displayName += *p;
                 ++p; // skip the '\'
                 if ( *p )
                   displayName += *p;
                 else
                   //return KPIM::UnexpectedEnd;
                   goto ABORT_PARSING;
                 break;
      case ',' : if ( !inQuotedString ) {
                   //if ( allowMultipleAddresses )
                   //  stop = true;
                   //else
                   //  return KPIM::UnexpectedComma;
                   goto ABORT_PARSING;
                 }
                 else
                   displayName += *p;
                 break;
      default :  displayName += *p;
      }
      break;
    }
    case InComment : {
      switch ( *p ) {
      case '(' : ++commentLevel;
                 comment += *p;
                 break;
      case ')' : --commentLevel;
                 if ( commentLevel == 0 ) {
                   context = TopLevel;
                   comment += ' '; // separate the text of several comments
                 }
                 else
                   comment += *p;
                 break;
      case '\\' : // quoted character
                 comment += *p;
                 ++p; // skip the '\'
                 if ( *p )
                   comment += *p;
                 else
                   //return KPIM::UnexpectedEnd;
                   goto ABORT_PARSING;
                 break;
      default :  comment += *p;
      }
      break;
    }
    case InAngleAddress : {
      switch ( *p ) {
      case '"' : inQuotedString = !inQuotedString;
                 addrSpec += *p;
                 break;
      case '>' : if ( !inQuotedString ) {
                   context = TopLevel;
                 }
                 else
                   addrSpec += *p;
                 break;
      case '\\' : // quoted character
                 addrSpec += *p;
                 ++p; // skip the '\'
                 if ( *p )
                   addrSpec += *p;
                 else
                   //return KPIM::UnexpectedEnd;
                   goto ABORT_PARSING;
                 break;
      default :  addrSpec += *p;
      }
      break;
    }
    } // switch ( context )
  }

ABORT_PARSING:
  displayName = displayName.trimmed();
  comment = comment.trimmed();
  addrSpec = addrSpec.trimmed();

  fullName = QString::fromUtf8( displayName );
  email = QString::fromUtf8( addrSpec );

  // check for errors
  if ( inQuotedString )
    return; // KPIM::UnbalancedQuote;
  if ( context == InComment )
    return; // KPIM::UnbalancedParens;
  if ( context == InAngleAddress )
    return; // KPIM::UnclosedAngleAddr;

  if ( addrSpec.isEmpty() ) {
    if ( displayName.isEmpty() )
      return; // KPIM::NoAddressSpec;
    else {
      //addrSpec = displayName;
      //displayName.truncate( 0 );
      // Address of the form "foo@bar" or "foo@bar (Name)".
      email = fullName;
      fullName = QString::fromUtf8( comment );
    }
  }

  // Check that we do not have any extra characters on the end of the
  // strings
  unsigned int len = fullName.length();
  if ( fullName[ 0 ] == '"' && fullName[ len - 1 ] == '"' )
    fullName = fullName.mid( 1, len - 2 );
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

void Addressee::setSortMode( KABC::SortMode *mode )
{
  mSortMode = mode;
}

bool Addressee::operator< ( const Addressee &addr ) const
{
  if ( !mSortMode )
    return false;
  else
    return mSortMode->lesser( *this, addr );
}

QDataStream &KABC::operator<<( QDataStream &s, const Addressee &a )
{
  if (!a.mData) return s;

  s << a.uid();

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

  s >> a.mData->uid;

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

bool matchBinaryPattern( int value, int pattern )
{
  /**
    We want to match all telephonnumbers/addresses which have the bits in the
    pattern set. More are allowed.
    if pattern == 0 we have a special handling, then we want only those with
    exactly no bit set.
   */
  if ( pattern == 0 )
    return ( value == 0 );
  else
    return ( pattern == ( pattern & value ) );
}

template <class L>
bool listEquals( const QList<L> &list, const QList<L> &pattern )
{
  if ( list.count() != pattern.count() )
    return false;

  for ( int i = 0; i < list.count(); ++i )
    if ( !pattern.contains( list[ i ] ) )
      return false;

  return true;
}

bool listEquals( const QStringList &list, const QStringList &pattern )
{
  if ( list.count() != pattern.count() )
    return false;

  for ( int i = 0; i < list.count(); ++i )
    if ( !pattern.contains( list[ i ] ) )
      return false;

  return true;
}

bool emailsEquals( const QStringList &list, const QStringList &pattern )
{
  if ( list.count() != pattern.count() )
    return false;

  if ( list.isEmpty() )
    return true;

  if ( list.first() != pattern.first() )
    return false;

  QStringList::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it )
    if ( !pattern.contains( *it ) )
      return false;

  return true;
}
