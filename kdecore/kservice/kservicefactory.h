/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2006 David Faure <faure@kde.org>

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

#ifndef __kservicefactory_h__
#define __kservicefactory_h__

#include <qstringlist.h>

#include "kserviceoffer.h"
#include "ksycocafactory.h"
#include <assert.h>

class KSycoca;
class KSycocaDict;

/**
 * @internal
 * A sycoca factory for services (e.g. applications)
 * It loads the services from parsing directories (e.g. applnk/)
 * but can also create service from data streams or single config files
 */
class KDECORE_EXPORT KServiceFactory : public KSycocaFactory
{
  K_SYCOCAFACTORY( KST_KServiceFactory )
public:
  /**
   * Create factory
   */
  KServiceFactory();
  virtual ~KServiceFactory();

  /**
   * Construct a KService from a config file.
   */
  virtual KSycocaEntry *createEntry(const QString &, const char *)
    { assert(0); return 0; }

  /**
   * Find a service (by translated name, e.g. "Terminal")
   */
  KService::Ptr findServiceByName( const QString &_name );

  /**
   * Find a service (by desktop file name, e.g. "konsole")
   */
  KService::Ptr findServiceByDesktopName( const QString &_name );

  /**
   * Find a service ( by desktop path, e.g. "System/konsole.desktop")
   */
  KService::Ptr findServiceByDesktopPath( const QString &_name );

  /**
   * Find a service ( by menu id, e.g. "kde-konsole.desktop")
   */
  KService::Ptr findServiceByMenuId( const QString &_menuId );

  /**
   * @return the services supporting the given service type
   * The @p serviceOffersOffset allows to jump to the right entries directly.
   */
  KServiceOfferList offers( int serviceTypeOffset, int serviceOffersOffset );

  /**
   * @return the services supporting the given service type; without information about initialPreference
   * The @p serviceOffersOffset allows to jump to the right entries directly.
   */
  KService::List serviceOffers( int serviceTypeOffset, int serviceOffersOffset );

  /**
   * Test if a specific service is associated with a specific servicetype
   * @param serviceTypeOffset the offset of the service type being tested
   * @param serviceOffersOffset allows to jump to the right entries for the service type directly.
   * @param testedServiceOffset the offset of the service being tested
   */
  bool hasOffer( int serviceTypeOffset, int serviceOffersOffset, int testedServiceOffset );

  /**
   * @return all services. Very memory consuming, avoid using.
   */
  KService::List allServices();

  /**
   * @return all services which have a "X-KDE-Init" line.
   */
  KService::List allInitServices();

  /**
   * @return the unique service factory, creating it if necessary
   */
  static KServiceFactory * self();

protected:
  virtual KService * createEntry(int offset);
  int m_offerListOffset;
  int m_initListOffset;
  KSycocaDict *m_nameDict;
  int m_nameDictOffset;
  KSycocaDict *m_relNameDict;
  int m_relNameDictOffset;
  KSycocaDict *m_menuIdDict;
  int m_menuIdDictOffset;

private:
  static KServiceFactory *_self;
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KServiceFactoryPrivate* d;
};

#endif
