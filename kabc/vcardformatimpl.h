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
#ifndef KABC_VCARDFORMATIMPL_H
#define KABC_VCARDFORMATIMPL_H

#include <qstring.h>

#include "format.h"
#include "address.h"
#include "addressee.h"

#include <VCard.h>

namespace KABC {

class AddressBook;

/*
  @short Implementation of vCard backend for address book.
  
  This class implements reading and writing of address book information using
  the vCard format. It requires the vCard lib from kdepim.
*/
class VCardFormatImpl {
  public:
    bool load( AddressBook *, Resource *, QFile * );
    bool save( const Addressee &, QFile * );

  protected:
    void addTextValue (VCARD::VCard *, VCARD::EntityType, const QString & );
    QString readTextValue( VCARD::ContentLine * );
    
    void addDateValue( VCARD::VCard *, VCARD::EntityType, const QDate & );
    QDate readDateValue( VCARD::ContentLine * );

    void addDateTimeValue( VCARD::VCard *, VCARD::EntityType, const QDateTime & );
    QDateTime readDateTimeValue( VCARD::ContentLine * );
    
    void addAddressValue( VCARD::VCard *, const Address & );
    Address readAddressValue( VCARD::ContentLine * );

    void addLabelValue( VCARD::VCard *, const Address & );

    void addTelephoneValue( VCARD::VCard *, const PhoneNumber & );
    PhoneNumber readTelephoneValue( VCARD::ContentLine * );

    void addNValue( VCARD::VCard *, const Addressee & );
    void readNValue( VCARD::ContentLine *, Addressee & );
    
    void addCustomValue( VCARD::VCard *, const QString & );

    void addAddressParam( VCARD::ContentLine *, int );
    int readAddressParam( VCARD::ContentLine * );

    void addGeoValue( VCARD::VCard *, VCARD::EntityType, const Geo & );
    Geo readGeoValue( VCARD::ContentLine * );
};

}

#endif
