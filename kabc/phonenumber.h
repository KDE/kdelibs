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
  a type. The following types are available:
  <pre>
  Home  Home number
  Work  Office number
  Msg   Messaging
  Pref  Preferred number
  Voice Voice
  Fax   Fax machine
  Cell  Cell phone
  Video Video phone
  Bbs   Mailbox
  Modem Modem
  Car   Car phone
  Isdn  ISDN connection
  Pcs   Personal Communication Service
  Pager Pager
  </pre>
*/
class PhoneNumber
{
  public:
    typedef QValueList<PhoneNumber> List;
  
    enum { Home = 1, Work = 2, Msg = 4, Pref = 8, Voice = 16, Fax = 32,
           Cell = 64, Video = 128, Bbs = 256, Modem = 512, Car = 1024,
           Isdn = 2048, Pcs = 4096, Pager = 8192 };

    /**
      Create an empty phone number object.
    */  
    PhoneNumber();
    /**
      Create a phonenumber object.
      
      @param number Number
      @param type Type as defined in enum.
    */
    PhoneNumber( const QString &number, int type = Home );
    ~PhoneNumber();
    
    bool operator==( const PhoneNumber & ) const;
    bool operator!=( const PhoneNumber & ) const;
    
    /**
      Set unique identifier.
    */
    void setId( const QString &id );
    /**
      Return unique identifier.
    */
    QString id() const;
    
    /**
      Set number.
    */
    void setNumber( const QString & );
    /**
      Return number.
    */
    QString number() const;
    
    /**
      Set type.
    */
    void setType( int );
    /**
      Return type.
    */
    int type() const;

    /**
      Return translated label for phone number depending on its type.
    */
    QString label() const;

    /**
      Return translated label for phone number type.
    */
    static QString label( int type );

  private:
    void init();
  
    QString mId;
  
    int mType;
    QString mNumber;
};

}

#endif
