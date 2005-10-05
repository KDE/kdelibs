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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KABC_FORMATFACTORY_H
#define KABC_FORMATFACTORY_H

#include <q3dict.h>
#include <qstring.h>

#include <kconfig.h>
#include <klibloader.h>

#include "format.h"

namespace KABC {

struct FormatInfo
{
  QString library;
  QString nameLabel;
  QString descriptionLabel;
};

/**
 * Class for loading format plugins.
 *
 * Example:
 *
 * \code
 * KABC::FormatFactory *factory = KABC::FormatFactory::self();
 *
 * QStringList list = factory->formats();
 * QStringList::Iterator it;
 * for ( it = list.begin(); it != list.end(); ++it ) {
 *   KABC::Format *format = factory->format( (*it) );
 *   // do something with format
 * }
 * \endcode
 */
class KABC_EXPORT FormatFactory
{
  public:
    
    /**
      Destructor.
     */
    ~FormatFactory();

    /**
     * Returns the global format factory.
     */
    static FormatFactory *self();

    /**
     * Returns a pointer to a format object or a null pointer
     * if format type doesn't exist.
     *
     * @param type   The type of the format, returned by formats()
     */
    Format *format( const QString &type );

    /**
     * Returns a list of all available format types.
     */
    QStringList formats();

    /**
     * Returns the info structure for a special type.
     */
    FormatInfo *info( const QString &type );

  protected:
    FormatFactory();

  private:
    KLibrary *openLibrary( const QString& libName );

    static FormatFactory *mSelf;

    Q3Dict<FormatInfo> mFormatList;
};

}
#endif
