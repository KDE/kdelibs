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

#ifndef __k_reg_factories_h__
#define __k_reg_factories_h__

#include <qstringlist.h>
#include <ksharedptr.h>

#include "kregistry.h"
#include "kservicetype.h"
#include "kservices.h"

class KServiceType;
class KService;
class KServiceTypeFactory;
class KServiceFactory;

typedef KSharedPtr<KServiceType> KServiceTypePtr;
typedef KSharedPtr<KService> KServicePtr;

/**
 * A registry entry containing a service type
 * (e.g. a mimetype)
 */
class KServiceTypeEntry : public KRegEntry
{
public:
  KServiceTypeEntry( KRegistry* _reg, const QString& _file, KServiceType *_mime, KServiceTypeFactory *factory );
  virtual ~KServiceTypeEntry();
  
  void save( QDataStream& _str ) const;

  KServiceTypePtr serviceType() { return m_pServiceType; }

protected:
  KServiceTypePtr m_pServiceType;
  KServiceTypeFactory *m_pFactory;
};

/**
 * A registry factory for service types (e.g. mimetypes)
 * It loads the service types from parsing directories (e.g. mimelnk/)
 * but can also create service types from data streams or single config files
 */
class KServiceTypeFactory : public QObject, public KRegFactory
{
  Q_OBJECT
  friend KServiceTypeEntry;
public:
  /**
   * Uses the default user- and system-paths
   */
  KServiceTypeFactory();
  KServiceTypeFactory( const QStringList& _path_list );
  virtual ~KServiceTypeFactory() { };

  virtual KRegEntry* create( KRegistry* _reg, const QString& _file, QDataStream& _str );
  virtual KRegEntry* create( KRegistry* _reg, const QString& _file, KSimpleConfig &_cfg );
  /**
   * @return the path for which this factory is responsible.
   *         Please note that the return value may not have a trailing '/'.
   */
  virtual const QStringList & pathList() const { return m_pathList; }

  void addEntryNotify( KServiceTypeEntry *entry ) { emit entryAdded( entry ); } 
  void removeEntryNotify( KServiceTypeEntry *entry ) { emit entryRemoved( entry ); }
  
signals:
  void entryAdded( KServiceTypeEntry *entry );
  void entryRemoved( KServiceTypeEntry *entry );
  
private:
  QStringList m_pathList;
};

/**
 * A registry entry containing a service 
 */
class KServiceEntry : public KRegEntry
{
public:
  KServiceEntry( KRegistry* _reg, const QString& _file, KService *_service, KServiceFactory *factory );
  virtual ~KServiceEntry();

  void save( QDataStream& _str ) const;

  KServicePtr service() { return m_pService; }
  
protected:
  KServicePtr m_pService;
  KServiceFactory *m_pFactory;
};


/**
 * A registry factory for services (e.g. applications)
 * It loads the services from parsing directories (e.g. applnk/)
 * but can also create service from data streams or single config files
 */
class KServiceFactory : public QObject, public KRegFactory
{
  Q_OBJECT
  friend KServiceEntry;
public:
  /**
   * Uses the default user- and system-paths
   */
  KServiceFactory();
  KServiceFactory( const QStringList& _path_list );
  virtual ~KServiceFactory() { };
  
  virtual KRegEntry* create( KRegistry* _reg, const QString& _file, QDataStream& _str );
  virtual KRegEntry* create( KRegistry* _reg, const QString& _file, KSimpleConfig &_cfg );
  /**
   * @return the path for which this factory is responsible.
   *         Please note that the return value may not have a trailing '/'.
   */
  virtual const QStringList & pathList() const { return m_pathList; }

  void addEntryNotify( KServiceEntry *entry ) { emit entryAdded( entry ); }
  void removeEntryNotify( KServiceEntry *entry ) { emit entryRemoved( entry ); }
  
signals:
  void entryAdded( KServiceEntry *entry );
  void entryRemoved( KServiceEntry *entry );
  
private:
  QStringList m_pathList;
};

#endif
