/*
    This file is part of libkabc.
    Copyright (c) 2003 Zack Rusin <zack@kde.org>

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

#ifndef VCARDFORMATPLUGIN2_H
#define VCARDFORMATPLUGIN2_H

#include "formatplugin.h"
#include "addressee.h"

class QFile;

namespace KABC {

  class Resource;
  class AddressBook;
  /**
     @short Interface of vCard backend for address book.

     This class implements the file format interface of address book entries for
     the vCard format.
  */
  class VCardFormatPlugin2 : public FormatPlugin
  {
  public:
    VCardFormatPlugin2();
    virtual ~VCardFormatPlugin2();

    bool load( Addressee &, QFile *file );
    bool loadAll( AddressBook *, Resource *, QFile *file );
    void save( const Addressee &, QFile *file );
    void saveAll( AddressBook *, Resource *, QFile *file );

    bool checkFormat( QFile *file ) const;

  private:
    struct VCardFormatPrivate;
    VCardFormatPrivate *d;
  };


}


#endif
