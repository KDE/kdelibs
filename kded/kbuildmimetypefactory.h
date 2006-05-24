/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>

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

#ifndef __k_build_mime_type_factory_h__
#define __k_build_mime_type_factory_h__

#include <kmimetypefactory.h>
#include <qstringlist.h>

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
  virtual KMimeType::Ptr findMimeTypeByName(const QString &_name);

  virtual KSycocaEntry::List allEntries();

  /**
   * Construct a KMimeType from a config file.
   */
  virtual KSycocaEntry * createEntry(const QString &file, const char *resource);

  virtual KMimeType * createEntry( int ) { assert(0); return 0L; }

  /**
   * Add entry
   */
  virtual void addEntry(const KSycocaEntry::Ptr& newEntry);

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
};

#endif
