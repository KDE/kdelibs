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

#include <kabc/addressee.h>

namespace KABC {

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

  VCardConverter();
  ~VCardConverter();
  
  bool VCardToAddressee( const QString &str, Addressee &addr, Version = v3_0 );
  bool AddresseeToVCard( const Addressee &addr, QString &str, Version = v3_0 );

private:
  struct VCardConverterData;
  VCardConverterData *d;
};

}
#endif
