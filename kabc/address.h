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

// template tags for address formatting localization
#define KABC_FMTTAG_realname   QString("%n")
#define KABC_FMTTAG_REALNAME   QString("%N")
#define KABC_FMTTAG_company    QString("%cm")
#define KABC_FMTTAG_COMPANY    QString("%CM")
#define KABC_FMTTAG_pobox      QString("%p")
#define KABC_FMTTAG_street     QString("%s")
#define KABC_FMTTAG_STREET     QString("%S")
#define KABC_FMTTAG_zipcode    QString("%z")
#define KABC_FMTTAG_location   QString("%l")
#define KABC_FMTTAG_LOCATION   QString("%L")
#define KABC_FMTTAG_region     QString("%r")
#define KABC_FMTTAG_REGION     QString("%R")
#define KABC_FMTTAG_newline    QString("\\n")
#define KABC_FMTTAG_condcomma  QString("%,")
#define KABC_FMTTAG_condwhite  QString("%w")
#define KABC_FMTTAG_purgeempty QString("%0")

namespace KABC {

/**
  @short Postal address information.
  
  This class represents information about a postal address.
*/
class Address
{
    friend QDataStream &operator<<( QDataStream &, const Address & );
    friend QDataStream &operator>>( QDataStream &, Address & );

  public:
    /**
      List of addresses.
    */
    typedef QValueList<Address> List;
    typedef QValueList<int> TypeList;
  
    /**
      Address types:
     
      @li @p Dom -    domestic
      @li @p Intl -   international
      @li @p Postal - postal
      @li @p Parcel - parcel
      @li @p Home -   home address
      @li @p Work -   address at work
      @li @p Pref -   preferred address
    */
    enum Type { Dom = 1, Intl = 2, Postal = 4, Parcel = 8, Home = 16, Work = 32,
           Pref = 64 };

    /**
      Constructor that creates an empty Address, which is initialized
      with a unique id (see @ref id()).
    */
    Address();
  
    /**
      This is like @ref Address() just above, with the difference
      that you can specify the type.
    */
    Address( int );

    bool operator==( const Address & ) const;
    bool operator!=( const Address & ) const;
  
    /**
      Returns true, if the address is empty.
    */
    bool isEmpty() const;

    /**
      Clears all entries of the address.
    */
    void clear();

    /**
      Sets the unique id.
    */
    void setId( const QString & );

    /*
      Returns the unique id.
    */
    QString id() const;

    /**
      Sets the type of address. See enum for definiton of types. 
     
      @param type type, can be a bitwise or of multiple types.
    */
    void setType( int type );

    /**
      Returns the type of address. Can be a bitwise or of multiple types.
    */
    int type() const;

    /**
      Returns a translated string of all types the address has.
    */
    QString typeLabel() const;

    /**
      Sets the post office box.
    */
    void setPostOfficeBox( const QString & );

    /**
      Returns the post office box.
    */
    QString postOfficeBox() const;

    /**
      Returns the translated label for post office box field.
    */
    static QString postOfficeBoxLabel();

    /**
      Sets the extended address information.
    */
    void setExtended( const QString & );

    /**
      Returns the extended address information.
    */
    QString extended() const;

    /**
      Returns the translated label for extended field.
    */
    static QString extendedLabel();
    
    /**
      Sets the street (including number).
    */
    void setStreet( const QString & );

    /**
      Returns the street.
    */
    QString street() const;

    /**
      Returns the translated label for street field.
    */
    static QString streetLabel();

    /**
      Sets the locality, e.g. city.
    */
    void setLocality( const QString & );

    /**
      Returns the locality.
    */
    QString locality() const;

    /**
      Returns the translated label for locality field.
    */
    static QString localityLabel();

    /**
      Sets the region, e.g. state.
    */
    void setRegion( const QString & );

    /**
      Returns the region.
    */
    QString region() const;

    /**
      Returns the translated label for region field.
    */
    static QString regionLabel();
 
    /**
      Sets the postal code.
    */
    void setPostalCode( const QString & );

    /**
      Returns the postal code.
    */
    QString postalCode() const;

    /**
      Returns the translated label for postal code field.
    */
    static QString postalCodeLabel();

    /**
      Sets the country.
    */
    void setCountry( const QString & );

    /**
      Returns the country.
    */
    QString country() const;

    /**
      Returns the translated label for country field.
    */
    static QString countryLabel();

    /**
      Sets the delivery label. This is the literal text to be used as label.
    */
    void setLabel( const QString & );

    /**
      Returns the delivery label.
    */
    QString label() const;

    /**
      Returns the translated label for delivery label field.
    */
    static QString labelLabel();

    /**
      Returns the list of available types.
    */
    static TypeList typeList();

    /**
      Returns the translated label for a special type.
    */
    static QString typeLabel( int type );

    /**
      Used for debug output.
    */
    void dump() const;

    /** 
      Returns this address formatted according to the country-specific
      address formatting rules. The formatting rules applied depend on 
      either the addresses {@link #country country} field, or (if the 
      latter is empty) on the system country setting. If companyName is
      provided, an available business address format will be preferred.
      
      @param realName   the formatted name of the contact
      @param orgaName   the name of the organization or company
      @return           the formatted address (containing newline characters)
    */
    QString formattedAddress( const QString &realName=QString::null
                            , const QString &orgaName=QString::null ) const;

    /**
      Returns ISO code for a localized country name. Only localized country
      names will be understood. This might be replaced by a KLocale method in
      the future.
      @param cname  name of the country
      @return       two digit ISO code
    */
    QString countryToISO( const QString &cname ) const;
    
  private:
    /** 
      Parses a snippet of an address template
      @param tsection   the template string to be parsed
      @param result     QString reference in which the result will be stored
      @return           true if at least one tag evaluated positively, else false
    */
    bool parseAddressTemplateSection( const QString &tsection
                                    ,       QString &result
                                    , const QString &realName
                                    , const QString &orgaName ) const;

    /** 
      Finds the balanced closing bracket starting from the opening bracket at 
      pos in tsection.
      @return  position of closing bracket, -1 for unbalanced brackets
    */
    int  findBalancedBracket( const QString &tsection, int pos ) const;

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

QDataStream &operator<<( QDataStream &, const Address & );
QDataStream &operator>>( QDataStream &, Address & );

}
#endif
// vim:tw=78 cin et sw=2 comments=sr\:/*,mb\:\ ,ex\:*/,\://
