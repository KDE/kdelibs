/*
    This file is part of libkresources.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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

#ifndef KRESOURCES_RESOURCEFACTORY_H
#define KRESOURCES_RESOURCEFACTORY_H

#include <qdict.h>
#include <qstring.h>

#include <kconfig.h>
#include <klibloader.h>

#include "resource.h"
#include "resourceconfigwidget.h"

namespace KRES {

struct ResourceInfo {
  QString library;
  QString nameLabel;
  QString descriptionLabel;
};

/**
 * Class for loading resource plugins.
 * Do not use this class directly. Use ResourceManager instead
 *
 * Example:
 *
 * <pre>
 * KABC::ResourceFactory<Calendar> *factory = KABC::ResourceFactory<Calendar>::self();
 *
 * QStringList list = factory->resources();
 * QStringList::Iterator it;
 * for ( it = list.begin(); it != list.end(); ++it ) {
 *   Resource<Calendar> *resource = factory->resource( (*it),
 *        KABC::StdAddressBook::self(), 0 );
 *   // do something with resource
 * }
 * </pre>
 */
class ResourceFactory
{
public:
    
  /**
   * Returns the global resource factory.
   */
  static ResourceFactory *self( const QString& resourceFamily );

  ~ResourceFactory();

  /**
   * Returns the config widget for the given resource type,
   * or a null pointer if resource type doesn't exist.
   *
   * @param type   The type of the resource, returned by @ref resources()
   * @param resource The resource to be editted. 
   * @param parent The parent widget
   */
  ResourceConfigWidget *configWidget( const QString& type, QWidget *parent = 0 );

  /**
   * Returns a pointer to a resource object or a null pointer
   * if resource type doesn't exist.
   *
   * @param type   The type of the resource, returned by @ref resources()
   * @param ab     The address book, the resource should belong to
   * @param config The config object where the resource get it settings from, or 0 if a new resource should be created.
   */
  Resource *resource( const QString& type, const KConfig *config );

  /**
   * Returns a list of all available resource types.
   */
  QStringList resourceTypeNames();

  /**
   * Returns the info structure for a special type.
   */
  ResourceInfo *info( const QString &type );

protected:
  ResourceFactory( const QString& resourceFamily );

private:
  KLibrary *openLibrary( const QString& libName );

  static QDict<ResourceFactory> *mSelves;

  QString mResourceFamily;
  QDict<ResourceInfo> mResourceList;
};

}
#endif
