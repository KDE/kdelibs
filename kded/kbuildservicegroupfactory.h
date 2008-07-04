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

#ifndef KDED_KBUILD_SERVICE_GROUP_FACTORY_H
#define KDED_KBUILD_SERVICE_GROUP_FACTORY_H

#include <kservice.h>
#include <kservicegroupfactory.h>
#include <QtCore/QStringList>

/**
 * Service group factory for building ksycoca
 * @internal
 */
class KBuildServiceGroupFactory : public KServiceGroupFactory
{
public:
  /**
   * Create factory
   */
  KBuildServiceGroupFactory();

  virtual ~KBuildServiceGroupFactory();

  /**
   * Create new entry.
   */
  virtual KServiceGroup * createEntry(const QString &, const char *) const;

  virtual KServiceGroup * createEntry(int) const { assert(0); return 0L; }

  /**
   * Adds the entry @p newEntry to the menu @p menuName
   */
  void addNewEntryTo( const QString &menuName, const KService::Ptr& newEntry);

  /**
   * Adds the entry @p newEntry to the "parent group" @p parent, creating
   * the group if necassery.
   * A "parent group" is a group of services that all have the same
   * "X-KDE-ParentApp".
   */
  void addNewChild( const QString &parent, const KSycocaEntry::Ptr& newEntry);

  /**
   * Add new menu @p menuName defined by @p file
   * When @p entry is non-null it is re-used, otherwise a new group is created.
   * A pointer to the group is returned.
   */
  KServiceGroup::Ptr addNew( const QString &menuName, const QString& file, KServiceGroup::Ptr entry, bool isDeleted);

  /**
   * Find a group ( by desktop path, e.g. "Applications/Editors")
   */
  virtual KServiceGroup::Ptr findGroupByDesktopPath( const QString &_name, bool deep = true );

  /**
   * Add a new menu entry
   */
  virtual void addEntry(const KSycocaEntry::Ptr& newEntry);

  /**
   * Write out servicegroup  specific index files.
   */
  virtual void save(QDataStream &str);

  /**
   * Write out header information
   */
  virtual void saveHeader(QDataStream &str);

  /**
   * Returns all resource types for this service factory
   */
  static QStringList resourceTypes();
};

#endif
