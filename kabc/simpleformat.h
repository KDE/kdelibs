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
#ifndef KABC_SIMPLEFORMAT_H
#define KABC_SIMPLEFORMAT_H
// $Id$

#include <qstring.h>

#include "format.h"

namespace KABC {

class AddressBook;

/*
  @short Simple KConfig based fileformat for address book entries.
  
  This class provides a simple file format for address book entries based on
  KSimpleConfig. It is not complete, i.e. it stores only a few data elements
  of the address book entries. Don't use it or finish it before using it.
*/
class SimpleFormat : public Format {
  public:
    bool load( AddressBook *, Resource *, const QString &fileName );
    bool save( AddressBook *, Resource *, const QString &fileName );
    void removeAddressee( const Addressee& addr );
    QString typeInfo() const;
    bool checkFormat( const QString &fileName ) const;
};

}

#endif
