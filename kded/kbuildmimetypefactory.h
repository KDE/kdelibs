/* This file is part of the KDE project
   Copyright 1999-2007 David Faure <faure@kde.org>

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

#ifndef KDED_KBUILD_MIME_TYPE_FACTORY_H
#define KDED_KBUILD_MIME_TYPE_FACTORY_H

#include <kmimetypefactory.h>
#include <QtCore/QStringList>
#include "kmimefileparser.h"

/**
 * Mime-type factory for building ksycoca
 * @internal
 */
class KBuildMimeTypeFactory : public KMimeTypeFactory
{
public:
  /**
   * Create factory
   */
  KBuildMimeTypeFactory();

  virtual ~KBuildMimeTypeFactory();

    /**
     * Find a mime type in the database file
     * @return a pointer to the mimetype in the memory dict (don't free!)
     */
    virtual KMimeType::Ptr findMimeTypeByName(const QString &_name, KMimeType::FindByNameOption options = KMimeType::DontResolveAlias);

  virtual KSycocaEntry::List allEntries() const;

  /**
   * Construct a KMimeType from a config file.
   */
  virtual KSycocaEntry * createEntry(const QString &file, const char *resource) const;

  virtual KMimeType * createEntry( int ) const { assert(0); return 0L; }

  /**
   * Write out mime type specific index files.
   */
  virtual void save(QDataStream &str);

  /**
   * Write out header information
   *
   * Don't forget to call the parent first when you override
   * this function.
   */
  virtual void saveHeader(QDataStream &str);

  /**
   * Returns all resource types for this factory
   */
  static QStringList resourceTypes();

private:
    void savePatternLists(QDataStream &str);
    KMimeFileParser m_parser;
    mutable QHash<QString, QString /*mainExtension*/> m_parsedMimeTypes;
};

#endif
