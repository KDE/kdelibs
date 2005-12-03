/* This file is part of the KDE project
   Copyright (C) 2000 Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __k_build_imageio_factory_h__
#define __k_build_imageio_factory_h__

#include <kimageiofactory.h>
#include <qstringlist.h>

/**
 * Service group factory for building ksycoca
 * @internal
 */
class KBuildImageIOFactory : public KImageIOFactory
{
public:
  /**
   * Create factory
   */
  KBuildImageIOFactory();

  virtual ~KBuildImageIOFactory();

  /**
   * Save header info to database
   */
  virtual void saveHeader(QDataStream &);

  /**
   * Write out service type specific index files.
   */
  virtual void save(QDataStream &str);

  /**
   * Create new entry.
   */
  virtual KSycocaEntry* createEntry(const QString &, const char *);

  virtual KSycocaEntry * createEntry( int ) { assert(0); return 0L; }

  virtual void addEntry(const KSycocaEntry::Ptr& newEntry);

  /**
   * Returns all resource types for this service factory
   */
  static QStringList resourceTypes();
};

#endif
