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

struct kde_loaded_l10nfile;

class KCataloguePrivate;

/**
 * This class abstracts a gettext message catalog. It will take care of
 * opening the file and reading the catalog.
 *
 * @see KLocale
 */
//REVISED: hausmann
class KCatalogue
{
public:
  /**
   * Constructor.
   *
   * @param name The name of the catalog
   * @param language The language of this catalog
   */
  explicit KCatalogue(const QString & name = QString::null, const QString & language = QString::null);

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
   * Returns the name of the catalog.
   *
   * @return The name of the catalog
   */
  QString name() const;

  /**
   * Returns the language of the catalog.
   *
   * @return The language of the catalog
   */
  QString language() const;

  /**
   * Returns the plural type for the catalog. This type is based on the language of the catalog
   and is cached for performance.
   *
   * @return The plural type for the catalog
   */
  int pluralType() const;

  /**
   * Sets the plural type for the catalog. The caller has probably looked it up in a kdelibs.mo-catalog
   * for the appropriate language
   *
   * @return The plural type for the catalog
   */
  void setPluralType( int pluralType );


  /**
   * Retrieves a translation of the specified message id.
   *
   * Do not pass 0 or "" strings as message ids.
   *
   * @param msgid The message id
   *
   * @return The translated message, in utf8 encoding, or 0 if not found
   */
  const char * translate( const char * msgid ) const;

private:
  /**
   * @internal Changes the current file name.
   *
   * @param fileName The new file name
   */

  void setFileName( const QString & fileName );
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
