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

/**
 * A registry entry containing a service type
 * (e.g. a mimetype)
 */
class KServiceTypeEntry : public KRegEntry
{
public:
  KServiceTypeEntry( KRegistry* _reg, const QString& _file, KServiceType *_mime );
  virtual ~KServiceTypeEntry();
  
  void save( QDataStream& _str ) const;

protected:
  KSharedPtr<KServiceType> m_pServiceType;
};

/**
 * A registry factory for service types (e.g. mimetypes)
 * It loads the service types from parsing directories (e.g. mimelnk/)
 * but can also create service types from data streams or single config files
 */
class KServiceTypeFactory : public KRegFactory
{
  friend KServiceTypeEntry;
  
public:
  /**
   * Uses the default user- and system-paths
   */
  KServiceTypeFactory();
  KServiceTypeFactory( const QStringList& _path_list );
  /**
   * @param _user_path defaults to _system_path if omitted.
   */
  KServiceTypeFactory( const QString& _system_path, const QString& _user_path = QString::null );
  virtual ~KServiceTypeFactory() { };

  virtual KRegEntry* create( KRegistry* _reg, const QString& _file, QDataStream& _str );
  virtual KRegEntry* create( KRegistry* _reg, const QString& _file, KSimpleConfig &_cfg );
  /**
   * @return the path for which this factory is responsible.
   *         Please note that the return value may not have a trailing '/'.
   */
  virtual QStringList pathList() const;

private:
  QStringList m_pathList;
};

/**
 * A registry entry containing a service 
 */
class KServiceEntry : public KRegEntry
{
public:
  KServiceEntry( KRegistry* _reg, const QString& _file, KService *_service );
  virtual ~KServiceEntry();

  void save( QDataStream& _str ) const;
  
protected:
  KService* m_pService;
};


/**
 * A registry factory for services (e.g. applications)
 * It loads the services from parsing directories (e.g. applnk/)
 * but can also create service from data streams or single config files
 */
class KServiceFactory : public KRegFactory
{
  friend KServiceEntry;
  
public:
  /**
   * Uses the default user- and system-paths
   */
  KServiceFactory();
  KServiceFactory( const QStringList& _path_list );
  /**
   * @param _user_path defaults to _system_path if omitted.
   */
  KServiceFactory( const QString& _system_path, const QString& _user_path = QString::null );
  virtual ~KServiceFactory() { };
  
  virtual KRegEntry* create( KRegistry* _reg, const QString& _file, QDataStream& _str );
  virtual KRegEntry* create( KRegistry* _reg, const QString& _file, KSimpleConfig &_cfg );
  /**
   * @return the path for which this factory is responsible.
   *         Please note that the return value may not have a trailing '/'.
   */
  virtual QStringList pathList() const;

private:
  QStringList m_pathList;
};

#endif
