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
#ifndef KABC_VCARDFORMAT_H
#define KABC_VCARDFORMAT_H

#include "format.h"

namespace KABC {

class AddressBook;
class Addressee;
class VCardFormatImpl;

/**
  @short Interface of vCard backend for address book.

  This class implements the file format interface of address book entries for
  the vCard format.
*/
class VCardFormat : public Format {
  public:
    VCardFormat();
    virtual ~VCardFormat();
  
    bool load( AddressBook *, Resource *, QFile *file );
    bool save( const Addressee &, QFile *file );
    bool checkFormat( QFile *file ) const;

  private:
    VCardFormatImpl *mImpl;
};

}

#endif
