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

#ifndef KABC_PHONENUMBER_H
#define KABC_PHONENUMBER_H

#include <qvaluelist.h>
#include <qstring.h>

namespace KABC {

/**
  @short Phonenumber information.
  
  This class provides phone number information. A phone number is classified by
  a type. The following types are available, it's possible to use multiple types
  @ref Types for a number by combining them through a logical or.
*/
class PhoneNumber
{
    friend QDataStream &operator<<( QDataStream &, const PhoneNumber & );
    friend QDataStream &operator>>( QDataStream &, PhoneNumber & );

  public:
    typedef QValueList<PhoneNumber> List;
    typedef QValueList<int> TypeList;

    /**
      @li @p Home -  Home number
      @li @p Work -  Office number
      @li @p Msg -   Messaging
      @li @p Pref -  Preferred number
      @li @p Voice - Voice
      @li @p Fax -   Fax machine
      @li @p Cell -  Cell phone
      @li @p Video - Video phone
      @li @p Bbs -   Mailbox
      @li @p Modem - Modem
      @li @p Car -   Car phone
      @li @p Isdn -  ISDN connection
      @li @p Pcs -   Personal Communication Service
      @li @p Pager - Pager
    */
    enum Types { Home = 1, Work = 2, Msg = 4, Pref = 8, Voice = 16, Fax = 32,
           Cell = 64, Video = 128, Bbs = 256, Modem = 512, Car = 1024,
           Isdn = 2048, Pcs = 4096, Pager = 8192 };

    /**
      Create an empty phone number object.
    */  
    PhoneNumber();

    /**
      Create a phonenumber object.
     
      @param number Number
      @param type   Type as defined in enum. Multiple types can be
                    specified by combining them by a logical or.
    */
    PhoneNumber( const QString &number, int type = Home );

    /**
      Destructor.
    */
    ~PhoneNumber();
    
    bool operator==( const PhoneNumber & ) const;
    bool operator!=( const PhoneNumber & ) const;
    
    /**
      Sets the unique identifier.
    */
    void setId( const QString &id );

    /**
      Returns the unique identifier.
    */
    QString id() const;
    
    /**
      Sets the number.
    */
    void setNumber( const QString & );

    /**
      Returns the number.
    */
    QString number() const;
    
    /**
      Sets the type. Multiple types can be specified by combining them by
      a logical or.
    */
    void setType( int );

    /**
      Returns the type. Can be a multiple types combined by a logical or.
    */
    int type() const;

    /**
      Returns a translated string of all types the address has.
    */
    QString typeLabel() const;

    /**
      Returns the translated label for phone number depending on its type.
    */
    QString label() const;

    /**
      Returns a list of all available types
    */
    static TypeList typeList();

    /**
      Returns the translated label for phone number type.
    */
    static QString typeLabel( int type );

    /**
      Returns the translated label for phone number type.
      @obsolete
    */
    static QString label( int type );

  private:
    void init();
  
    QString mId;
  
    int mType;
    QString mNumber;
};

QDataStream &operator<<( QDataStream &, const PhoneNumber & );
QDataStream &operator>>( QDataStream &, PhoneNumber & );

}

#endif
