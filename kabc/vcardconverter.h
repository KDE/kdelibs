/*
    This file is part of libkabc.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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

#ifndef KABC_VCARDCONVERTER_H
#define KABC_VCARDCONVERTER_H

#include <qstring.h>

#include "addressee.h"

namespace KABC {

/**
 * Class to convert a vcard string to a addressee and vice versa.
 * At the moment there exists read support for vCard2.1 and vCard3.0
 * and write support for vCard3.0
 */
class VCardConverter
{
public:

  /**
   * @li v2_1 - VCard format version 2.1
   * @li v3_0 - VCard format version 3.0
   */
  enum Version
  {
    v2_1,
    v3_0
  };

  /**
   * Constructor.
   */
  VCardConverter();

  /**
   * Destructor.
   */
  ~VCardConverter();
  
  /**
   * Converts a vcard string to an addressee.
   *
   * @param str     The vcard string.
   * @param addr    The addressee.
   * @param version The version of the vcard string.
   */
  bool vCardToAddressee( const QString &str, Addressee &addr, Version version = v3_0 );

  /**
   * Converts an addressee to a vcard string.
   *
   * @param addr    The addressee.
   * @param str     The vcard string.
   * @param version The version of the vcard string.
   */
  bool addresseeToVCard( const Addressee &addr, QString &str, Version version = v3_0 );

private:
  struct VCardConverterData;
  VCardConverterData *d;
};


/**
    Helper functions
  */

/**
  * Converts a QDateTime to a date string as it is used in VCard and LDIF files.
  * The return value is in the form "yyyyMMddThhmmss" (e.g. "20031201T120000")
  * @param dateTime date and time to be converted 
  * @since 3.2
  */
QString dateToVCardString( const QDateTime &dateTime );

/**
  * Converts a QDate to a short date string as it is used in VCard and LDIF files.
  * The return value is in the form "yyyyMMdd" (e.g. "20031201")
  * @param date date to be converted 
  * @since 3.2
  */
QString dateToVCardString( const QDate &date );

/**
  * Converts a date string as it is used in VCard and LDIF files to a QDateTime value.
  * If the date string does not contain a time value, it will be returned as 00:00:00.
  * (e.g. "20031201T120000" will return a QDateTime for 2003-12-01 at 12:00)
  * @param dateString string representing the date and time.
  * @since 3.2
  */
QDateTime VCardStringToDate( const QString &dateString );

}
#endif
