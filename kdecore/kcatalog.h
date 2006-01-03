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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KCATALOG_H
#define KCATALOG_H

#include <qstring.h>
#include <kdelibs_export.h>

class KCatalogPrivate;

/**
 * This class abstracts a gettext message catalog. It will take care of
 * needed gettext bindings.
 *
 * @see KLocale
 */
//REVISED: hausmann
class KDECORE_EXPORT KCatalog
{
public:
  /**
   * Constructor.
   *
   * @param name The name of the catalog
   * @param language The language of this catalog
   */
  explicit KCatalog( const QString &name, const QString &language );

  /**
   * Copy constructor.
   */
  KCatalog(const KCatalog & rhs);

  /**
   * Assignment operator.
   */
  KCatalog & operator = ( const KCatalog & rhs);

  /**
   * Destructor.
   */
  virtual ~KCatalog();

  /**
   * Finds the locale directory for the given catalog in given language.
   *
   * @param name The name of the catalog
   * @param language The language of this catalog
   *
   * @return The locale directory if found, QString() otherwise.
   */
  static QString catalogLocaleDir( const QString &name,
                                   const QString &language );

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
   * Returns locale directory of the catalog.
   *
   * @return The locale directory of the catalog.
   */
  QString localeDir() const;

  /**
   * Retrieves a translation of the specified message id.
   *
   * Do not pass 0 or "" strings as message id.
   *
   * @param msgid The message id
   *
   * @return The translated message, or @p msgid if not found
   */
  QString translate( const char * msgid ) const;

  /**
   * Retrieves a translation of the specified message id with given context.
   *
   * Do not pass 0 or "" strings as message id or context.
   *
   * @param msgctxt The context
   * @param msgid The message id
   *
   * @return The translated message, or @p msgid if not found
   */
  QString translate( const char * msgctxt, const char * msgid ) const;

  /**
   * Retrieves a proper plural form of translation for the specified English
   * singular and plural message ids.
   *
   * Do not pass 0 or "" strings as message ids.
   *
   * @param msgid The singular message id
   * @param msgid_plural The plural message id
   * @param n The number to which the plural form applies
   *
   * @return The translated message, or proper English form if not found
   */
  QString translate( const char * msgid, const char * msgid_plural,
                     unsigned long n ) const;

  /**
   * Retrieves a proper plural form of translation for the specified English
   * singular and plural message ids, with given context.
   *
   * Do not pass 0 or "" strings as message ids or context.
   *
   * @param msgctxt The context
   * @param msgid The singular message id
   * @param msgid_plural The plural message id
   * @param n The number to which the plural form applies
   *
   * @return The translated message, or proper English form if not found
   */
  QString translate( const char * msgctxt, const char * msgid,
                     const char * msgid_plural, unsigned long n ) const;

private:
  KCatalogPrivate * d;
};

#endif
