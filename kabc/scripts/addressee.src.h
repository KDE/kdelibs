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

#ifndef KABC_ADDRESSEE_H
#define KABC_ADDRESSEE_H

#include <qdatetime.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>

#include <ksharedptr.h>
#include <kurl.h>

#include "address.h"
#include "agent.h"
#include "geo.h"
#include "key.h"
#include "phonenumber.h"
#include "picture.h"
#include "secrecy.h"
#include "sound.h"
#include "timezone.h"

namespace KABC {

class Resource;

/**
  @short address book entry
 
  This class represents an entry in the address book.
 
  The data of this class is implicitly shared. You can pass this class by value.
 
  If you need the name of a field for presenting it to the user you should use
  the functions ending in Label(). They return a translated string which can be
  used as label for the corresponding field.
 
  About the name fields:
 
  givenName() is the first name and familyName() the last name. In some 
  countries the family name comes first, that's the reason for the 
  naming. formattedName() is the full name with the correct formatting.
  It is used as an override, when the correct formatting can't be generated
  from the other name fields automatically.
 
  realName() returns a fully formatted name(). It uses formattedName, if set,
  otherwise  it constucts the name from the name fields. As fallback, if
  nothing  else is set it uses name().
 
  name() is the NAME type of RFC2426. It can be used as internal name for the
  data enty, but shouldn't be used for displaying the data to the user.
 */
class Addressee
{
    friend QDataStream &operator<<( QDataStream &, const Addressee & );
    friend QDataStream &operator>>( QDataStream &, Addressee & );

  public:
    typedef QValueList<Addressee> List;

    /**
      Construct an empty address book entry.
     */
    Addressee();
    ~Addressee();

    Addressee( const Addressee & );
    Addressee &operator=( const Addressee & );

    bool operator==( const Addressee & ) const;
    bool operator!=( const Addressee & ) const;

    /**
      Return, if the address book entry is empty.
     */
    bool isEmpty() const;

    --DECLARATIONS--
    /**
      Set name fields by parsing the given string and trying to associate the
      parts of the string with according fields. This function should probably
      be a bit more clever.
     */
    void setNameFromString( const QString & );

    /**
      Return the name of the addressee. This is calculated from all the name
      fields.
     */
    QString realName() const;
    
    /**
      Return email address including real name.
      
      @param email Email address to be used to construct the full email string.
                   If this is QString::null the preferred email address is used.
     */
    QString fullEmail( const QString &email=QString::null ) const;
    
    /**
      Insert an email address. If the email address already exists in this
      addressee it is not duplicated.
      
      @param email Email address
      @param preferred Set to true, if this is the preferred email address of
                       the addressee.
     */
    void insertEmail( const QString &email, bool preferred=false );

    /**
      Remove email address. If the email address doesn't exist, nothing happens.
     */
    void removeEmail( const QString &email );

    /**
      Return preferred email address. This is the first email address or the
      last one added with @ref isnertEmail() with a set preferred parameter.
     */
    QString preferredEmail() const;

    /**
      Return list of all email addresses.
     */
    QStringList emails() const;
    
    /**
      Insert a phone number. If a phoen number with the same id already exists
      in this addressee it is not duplicated.
     */
    void insertPhoneNumber( const PhoneNumber &phoneNumber );

    /**
      Remove phone number. If no phone number with the given id exists for this
      addresse nothing happens.
     */
    void removePhoneNumber( const PhoneNumber &phoneNumber );

    /**
      Return phone number, which matches the given type.
     */
    PhoneNumber phoneNumber( int type ) const;

    /**
      Return list of all phone numbers.
     */
    PhoneNumber::List phoneNumbers() const;

    /**
      Return list of phone numbers with a special type.
     */
    PhoneNumber::List phoneNumbers( int type ) const;

    /**
      Return phone number with the given id.
     */
    PhoneNumber findPhoneNumber( const QString &id ) const;

    /**
      Insert a key. If a key with the same id already exists
      in this addressee it is not duplicated.
     */
    void insertKey( const Key &key );

    /**
      Remove a key. If no key with the given id exists for this
      addresse nothing happens.
     */
    void removeKey( const Key &key );

    /**
      Return key, which matches the given type.
      If @param type == Key::Custom you can specify a string
      that should match. If you leave the string empty, the first
      key with a custom value is returned.
     */
    Key key( int type, QString customTypeString = QString::null ) const;

    /**
      Return list of all keys.
     */
    Key::List keys() const;

    /**
      Return list of keys with a special type.
      If @param type == Key::Custom you can specify a string
      that should match. If you leave the string empty, all custom
      keys will be returned.
     */
    Key::List keys( int type, QString customTypeString = QString::null  ) const;

    /**
      Return key with the given id.
     */
    Key findKey( const QString &id ) const;
    
    /**
      Insert an address. If an address with the same id already exists
      in this addressee it is not duplicated.
     */
    void insertAddress( const Address &address );

    /**
      Remove address. If no address with the given id exists for this
      addresse nothing happens.
     */
    void removeAddress( const Address &address );

    /**
      Return address, which matches the given type.
     */
    Address address( int type ) const;

    /**
      Return list of all addresses.
     */
    Address::List addresses() const;

    /**
      Return list of addresses with a special type.
     */
    Address::List addresses( int type ) const;
	
    /**
      Return address with the given id.
     */
    Address findAddress( const QString &id ) const;

    /**
      Insert category. If the category already exists it is not dulicated.
     */
    void insertCategory( const QString & );

    /**
      Remove category.
     */
    void removeCategory( const QString & );

    /**
      Return, if addressee has the given category.
     */
    bool hasCategory( const QString & ) const;

    /**
      Set categories to given value.
     */
    void setCategories( const QStringList & );

    /**
      Return list of all set categories.
     */
    QStringList categories() const;

    /**
      Insert custom entry. The entry is identified by the name of the inserting
      application and a unique name. If an entry with the given app and name
      already exists its value is replaced with the ne given value.
     */
    void insertCustom( const QString &app, const QString &name,
                       const QString &value );

    /**
      Remove custom entry.
     */
    void removeCustom( const QString &app, const QString &name );

    /**
      Return value of custom entry, identified by app and entry name.
     */
    QString custom( const QString &app, const QString &name ) const;

    /**
      Set all custom entries.
     */
    void setCustoms( const QStringList & );

    /**
      Return list of all custom entries.
     */
    QStringList customs() const;

    /**
      Parse full email address. The result is given back in fullName and email.
     */
    static void parseEmailAddress( const QString &rawEmail, QString &fullName,
                                   QString &email );

    /**
      Debug output.
     */
    void dump() const;

    /**
      Returns string representation of the addressee.
     */
    QString asString() const;

    /**
      Set resource where the addressee is from.
     */
    void setResource( Resource *resource );

    /**
      Return pointer to resource.
     */
    Resource *resource() const;
  
    /**
	    Mark addressee as changed.
	   */
    void setChanged( bool value );

  	/**
	    Return whether the addressee is changed.
	   */
	  bool changed() const;

  private:
    Addressee copy();
    void detach();

    struct AddresseeData;
    KSharedPtr<AddresseeData> mData;
};

QDataStream &operator<<( QDataStream &, const Addressee & );
QDataStream &operator>>( QDataStream &, Addressee & );

}

#endif
