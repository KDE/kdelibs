/* This file is part of the KDE libraries
   Copyright (c) 2001 Hans Petter Bieker <bieker@kde.org>

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

#ifndef KCATALOGUE_H
#define KCATALOGUE_H

#include <qstring.h>

struct loaded_l10nfile;

class KCataloguePrivate;

/**
 * This class abstracs a gettext message catalogue. It will take care of
 * opening the file and reading the catalogue.
 *
 * @sa KLocale
 */
class KCatalogue
{
public:
  /**
   * Constructor.
   *
   * @param name The name of the catalogue
   */
  explicit KCatalogue(const QString & name = QString::null);

  /**
   * Copy constructor.
   */
  KCatalogue(const KCatalogue & rhs);

  /**
   * Assignment operator.
   */
  KCatalogue & operator = ( const KCatalogue & rhs);

  /**
   * Destructor.
   */
  virtual ~KCatalogue();

  /**
   * Retrieves the name of the catalogue.
   *
   * @return The name of the catalogue
   */
  QString name() const;

  /**
   * Changes the current file name.
   *
   * @param fileName The new file name
   */

  void setFileName( const QString & fileName );

  /**
   * Retrieves a translation of the specified message id.
   *
   * Do not pass 0 or "" strings as message ids.
   *
   * @param msgid The message id
   *
   * @return The translated message, or 0 if not found
   */
  const char * translate( const char * msgid ) const;

private:
  /**
   * @internal Retrieves the current file name.
   *
   * @return The current file name, if any.
   */
  QString fileName() const;

  /**
   * @internal Unloads the current file.
   */
  void doUnload();

private:
  KCataloguePrivate * d;
};

#endif
