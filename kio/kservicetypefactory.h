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

#ifndef __k_service_type_factory_h__
#define __k_service_type_factory_h__

#include <qobject.h>
#include <qstringlist.h>

#include "ksycocafactory.h"
#include "kmimetype.h"

class KSycoca;
class KSycocaDict;

class KServiceType;
class KFolderType;
class KDEDesktopMimeType;
class KExecMimeType;

/**
 * @internal
 * A sycoca factory for service types (e.g. mimetypes)
 * It loads the service types from parsing directories (e.g. mimelnk/)
 * but can also create service types from data streams or single config files
 */
class KServiceTypeFactory : public QObject, public KSycocaFactory
{
  Q_OBJECT
  K_SYCOCAFACTORY( KST_KServiceTypeFactory );
public:
  /**
   * Create factory
   */
  KServiceTypeFactory();
  
  virtual ~KServiceTypeFactory();

  /**
   * Construct a KServiceType from a config file.
   */
  virtual KSycocaEntry *createEntry(const QString &file);

  /**
   * Find a service type in the database file (allocates it)
   */
  KServiceType * findServiceTypeByName( const QString &_name);

  /**
   * Find a service type in memory (returns a pointer to it)
   */
  KServiceType * findServiceTypeRef(const QString &_name);

  /**
   * @return all mimetypes
   * Slow and memory consuming, avoid using
   */
  KMimeType::List allMimeTypes();
  
  /**
   * @return true if at least one mimetype is present
   * Safety test
   */
  bool checkMimeTypes();

  /**
   * @return the unique servicetype factory, creating it if necessary
   */
  static KServiceTypeFactory * self();

protected:
  KServiceType *createServiceType(int offset);

private:
  static KServiceTypeFactory *_self;
};

#endif
