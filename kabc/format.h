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
#ifndef KABC_FORMAT_H
#define KABC_FORMAT_H

#include <qstring.h>

#include "resource.h"

#define FORMAT_VCARD	0
#define FORMAT_BINARY	1

namespace KABC {

class AddressBook;

/**
  @short Base class for address book formats.

  This class provides an abstract interface for address book file formats.
*/
class Format {
  public:
    /**
      Load addressbook from file.
    */
    virtual bool load( AddressBook *, Resource *, const QString &fileName ) = 0;
    /**
      Save addressbook to file.
    */
    virtual bool save( AddressBook *, Resource *, const QString &fileName ) = 0;
	
    /**
      Remove a addressee from file.
    */
    virtual void removeAddressee( const Addressee& addr ) = 0;

    /**
      String representation of this format.
    */
    virtual QString typeInfo() const = 0;
};

}

#endif
