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

#ifndef KABC_FORMAT_H
#define KABC_FORMAT_H

#include <qfile.h>

#include "plugin.h"
#include "resource.h"

namespace KABC {

class AddressBook;
class Addressee;

/**
 * @short Base class for address book formats.
 *
 * This class provides an abstract interface for ResourceFile and
 * ResourceDir formats.
 *
 * @internal
 */
class Format : public Plugin
{
public:

  /**
   * Load single addressee from file.
   */
  virtual bool load( Addressee &, QFile *file ) = 0;

  /**
   * Load whole addressbook from file.
   */
  virtual bool loadAll( AddressBook *, Resource *, QFile *file ) = 0;

  /**
   * Save a single Addressee to file.
   */
  virtual void save( const Addressee &, QFile *file ) = 0;
    
  /**
   * Save whole addressbook to file.
   */
  virtual void saveAll( AddressBook *, Resource *, QFile *file ) = 0;
    
  /**
   * Checks if given file contains the right format
   */
  virtual bool checkFormat( QFile *file ) const = 0;
};

}
#endif
