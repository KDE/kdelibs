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

#include <qfile.h>

#include "resource.h"

namespace KABC {

class AddressBook;
class Addressee;

/**
  @short Base class for address book formats.
 
  This class provides an abstract interface for ResourceFile formats.
 
  @internal
*/
class Format
{
  public:

    /**
      @li @p VCard -  VCard format according to rfc2426
      @li @p Binary - Binary format
      @li @p Simple - Simple format stored with KConfig
    */
    enum Type
    {
      VCard,
      Binary,
      Simple
    };

    /**
      Load addressbook from file.
    */
    virtual bool load( AddressBook *, Resource *, QFile *file ) = 0;

    /**
      Load addressbook from file.
    */
    bool load( AddressBook *, Resource *, const QString &fileName );

    /**
      Save a single Addressee to file.
    */
    virtual bool save( const Addressee &, QFile *file ) = 0;

    /**
      Save addressbook to file.
    */
    bool save( AddressBook *, Resource *, const QString &fileName );

    /**
      Checks if given file contains the right format
    */
    virtual bool checkFormat( QFile *file ) const = 0;
    bool checkFormat( const QString &fileName ) const;

    QString typeInfo();

    void removeAddressee( const Addressee & );
};

}
#endif
