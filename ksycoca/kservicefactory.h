/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 
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

#ifndef __kservicefactory_h__
#define __kservicefactory_h__

#include <qobject.h>
#include <qstringlist.h>
#include <ksharedptr.h>

#include "ksycocafactory.h"

class KSycoca;
class KSycocaDict;
class KService;
class KServiceList;

/**
 * A registry factory for services (e.g. applications)
 * It loads the services from parsing directories (e.g. applnk/)
 * but can also create service from data streams or single config files
 */
class KServiceFactory : public QObject, public KSycocaFactory
{
  Q_OBJECT
  K_SYCOCAFACTORY( KST_KServiceFactory );
public:
  /**
   * Create factory
   */
  KServiceFactory(bool buildDatabase = false);
  virtual ~KServiceFactory() { };
  
  /**
   * @return the path for which this factory is responsible.
   *         Please note that the return value may not have a trailing '/'.
   */
  virtual const QStringList * pathList() const { return m_pathList; }

  void add(KService *newEntry);
  static KService *findServiceByName( const QString &_name);

protected:
  KService *_findServiceByName( const QString &_name);
  KService *createService(int offset);

private:
  QStringList *m_pathList;
  KServiceList *m_entryList;
  KSycocaDict *m_entryDict;
  static KServiceFactory *self;
};

#endif
