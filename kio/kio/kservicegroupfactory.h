/* This file is part of the KDE project
   Copyright (C) 2000 Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __kservicegroupfactory_h__
#define __kservicegroupfactory_h__

#include <qstringlist.h>

#include "kservicegroup.h"
#include "ksycocafactory.h"
#include <assert.h>

class KSycoca;
class KSycocaDict;

/**
 * @internal
 * A sycoca factory for service groups (e.g. list of applications)
 * It loads the services from parsing directories (e.g. applnk/)
 */
class KServiceGroupFactory : public KSycocaFactory
{
  K_SYCOCAFACTORY( KST_KServiceGroupFactory )
public:
  /**
   * Create factory
   */
  KServiceGroupFactory();
  virtual ~KServiceGroupFactory();

  /**
   * Construct a KServiceGroup from a config file.
   */
  virtual KSycocaEntry *createEntry(const QString &, const char *)
    { assert(0); return 0; }

  /**
   * Find a group ( by desktop path, e.g. "Applications/Editors")
   */
  KServiceGroup * findGroupByDesktopPath( const QString &_name, bool deep = true );

  /**
   * Find a base group by name, e.g. "settings"
   */
  KServiceGroup * findBaseGroup( const QString &_baseGroupName, bool deep = true );

  /**
   * @return the unique service group factory, creating it if necessary
   */
  static KServiceGroupFactory * self();
protected:
  KServiceGroup* createGroup(int offset, bool deep);
  KServiceGroup* createEntry(int offset);
  KSycocaDict *m_baseGroupDict;
  int m_baseGroupDictOffset;

private:
  static KServiceGroupFactory *_self;
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KServiceGroupFactoryPrivate* d;
};

#endif
