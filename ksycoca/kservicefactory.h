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
  KServiceFactory();
  virtual ~KServiceFactory();
  
  /**
   * Construct a KService from a config file.
   */
  virtual KSycocaEntry *createEntry(const QString &file);

  /**
   * Find a service
   */
  static KService *findServiceByName( const QString &_name );

  /**
   * @return the services supporting the given service type
   */
  static KServiceList *offers( int serviceTypeOffset );

protected:
  KService *_findServiceByName( const QString &_name);
  KServiceList *_offers( int serviceTypeOffset );
  KService *createService(int offset);

private:
  static KServiceFactory *self;
};

#endif
