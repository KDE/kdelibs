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
#ifndef KABC_ADDRESS_H
#define KABC_ADDRESS_H

#include <qstring.h>
#include <qvaluelist.h>

namespace KABC {

/*
  @short Postal address information.
  
  This class represents information about a postal address.
*/
class Address
{
  public:
    /**
      List of addresses.
    */
    typedef QValueList<Address> List;
  
    /**
      Address type:
      Dom domestic
      Intl international
      postal postal
      parcel parcel
      home home address
      work address at work
      pref preferred address
    */
    enum Type { Dom = 1, Intl = 2, Postal = 4, Parcel = 8, Home = 16, Work = 32,
           Pref = 64 };

    /**
      Construct an empty address. It is initialized with a unique id (@see
      id()).
    */
    Address();
  
    /**
      Construct an empty address of the given type. It is initialized with a
      n unique id (@see id()).
    */
    Address( int );

    bool operator==( const Address & ) const;
    bool operator!=( const Address & ) const;
  
    /**
      Return, if the address is empty.
    */
    bool isEmpty() const;

    /**
      Clear all entries of the address.
    */
    void clear();

    /**
      Set unique id.
    */
    void setId( const QString & );
    /*
      Return unique id.
    */
    QString id() const;

    /**
      Set type of address. See enum for definiton of types. 
    
      @param type type, can be a bitwise or of multiple types.
    */
    void setType( int type );
    /**
      Return type of address. Can be a bitwise or of multiple types.
    */
    int type() const;

    /**
      Set post office box.
    */
    void setPostOfficeBox( const QString & );
    /**
      Return post office box.
    */
    QString postOfficeBox() const;
    /**
      Return translated label for post office box field.
    */
    QString postOfficeBoxLabel() const;

    /**
      Set extended address information.
    */
    void setExtended( const QString & );
    /**
      Return extended address information.
    */
    QString extended() const;
    /**
      Return translated label for extended field.
    */
    QString extendedLabel() const;
    
    /**
      Set street (including number).
    */
    void setStreet( const QString & );
    /**
      Return street.
    */
    QString street() const;
    /**
      Return translated label for street field.
    */
    QString streetLabel() const;

    /**
      Set locality, e.g. city.
    */
    void setLocality( const QString & );
    /**
      Return locality.
    */
    QString locality() const;
    /**
      Return translated label for locality field.
    */
    QString localityLabel() const;

    /**
      Set region, e.g. state.
    */
    void setRegion( const QString & );
    /**
      Return region.
    */
    QString region() const;
    /**
      Return translated label for region field.
    */
    QString regionLabel() const;
 
    /**
      Set postal code.
    */
    void setPostalCode( const QString & );
    /**
      Return postal code.
    */
    QString postalCode() const;
    /**
      Return translated label for postal code field.
    */
    QString postalCodeLabel() const;

    /**
      Set country.
    */
    void setCountry( const QString & );
    /**
      Return country.
    */
    QString country() const;
    /**
      Return translated label for country field.
    */
    QString countryLabel() const;

    /**
      Set delivery label. This is the literal text to be used as label.
    */
    void setLabel( const QString & );
    /**
      Return delivery label.
    */
    QString label() const;
    /**
      Return translated label for delivery label field.
    */
    QString labelLabel() const;

    /**
      Debug output.
    */
    void dump() const;
  
  private:
    bool mEmpty;
  
    QString mId;
    int mType;
  
    QString mPostOfficeBox;
    QString mExtended;
    QString mStreet;
    QString mLocality;
    QString mRegion;
    QString mPostalCode;
    QString mCountry;
    QString mLabel;
};

}

#endif
