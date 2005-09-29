/*
    This file is part of libkresources.

    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include <kdebug.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kstaticdeleter.h>

#include <qfile.h>

#include "resource.h"
#include "factory.h"

using namespace KRES;

QDict<Factory> *Factory::mSelves = 0;
static KStaticDeleter< QDict<Factory> > staticDeleter;

Factory *Factory::self( const QString& resourceFamily )
{
  kdDebug(5650) << "Factory::self()" << endl;

  Factory *factory = 0;
  if ( !mSelves )
    staticDeleter.setObject( mSelves, new QDict<Factory> );

  factory = mSelves->find( resourceFamily );

  if ( !factory ) {
    factory = new Factory( resourceFamily );
    mSelves->insert( resourceFamily, factory );
  }

  return factory;
}

Factory::Factory( const QString& resourceFamily ) :
  mResourceFamily( resourceFamily )
{
  KTrader::OfferList plugins = KTrader::self()->query( "KResources/Plugin", QString( "[X-KDE-ResourceFamily] == '%1'" )
                                                .arg( resourceFamily ) );
  KTrader::OfferList::ConstIterator it;
  for ( it = plugins.begin(); it != plugins.end(); ++it ) {
    QVariant type = (*it)->property( "X-KDE-ResourceType" );
    if ( !type.toString().isEmpty() )
      mTypeMap.insert( type.toString(), *it );
  }
}

Factory::~Factory()
{
}

QStringList Factory::typeNames() const
{
  return mTypeMap.keys();
}

ConfigWidget *Factory::configWidget( const QString& type, QWidget *parent )
{
  if ( type.isEmpty() || !mTypeMap.contains( type ) )
    return 0;

  KService::Ptr ptr = mTypeMap[ type ];
  KLibFactory *factory = KLibLoader::self()->factory( ptr->library().latin1() );
  if ( !factory ) {
    kdDebug(5650) << "KRES::Factory::configWidget(): Factory creation failed "
                  << KLibLoader::self()->lastErrorMessage() << endl;
    return 0;
  }

  PluginFactoryBase *pluginFactory = static_cast<PluginFactoryBase *>( factory );

  if ( !pluginFactory ) {
    kdDebug(5650) << "KRES::Factory::configWidget(): no plugin factory."
                  << endl;
    return 0;
  }

  ConfigWidget *wdg = pluginFactory->configWidget( parent );
  if ( !wdg ) {
    kdDebug(5650) << "'" << ptr->library() << "' doesn't provide a ConfigWidget" << endl;
    return 0;
  }

  return wdg;
}

QString Factory::typeName( const QString &type ) const
{
  if ( type.isEmpty() || !mTypeMap.contains( type ) )
    return QString();

  KService::Ptr ptr = mTypeMap[ type ];
  return ptr->name();
}

QString Factory::typeDescription( const QString &type ) const
{
  if ( type.isEmpty() || !mTypeMap.contains( type ) )
    return QString();

  KService::Ptr ptr = mTypeMap[ type ];
  return ptr->comment();
}

Resource *Factory::resource( const QString& type, const KConfig *config )
{
  kdDebug(5650) << "Factory::resource( " << type << ", config )" << endl;

  if ( type.isEmpty() || !mTypeMap.contains( type ) ) {
    kdDebug(5650) << "Factory::resource() no such type " << type << endl;
    return 0;
  }

  KService::Ptr ptr = mTypeMap[ type ];
  KLibFactory *factory = KLibLoader::self()->factory( ptr->library().latin1() );
  if ( !factory ) {
    kdDebug(5650) << "KRES::Factory::resource(): Factory creation failed "
                  << KLibLoader::self()->lastErrorMessage() << endl;
    return 0;
  }

  PluginFactoryBase *pluginFactory = static_cast<PluginFactoryBase *>( factory );

  if ( !pluginFactory ) {
    kdDebug(5650) << "KRES::Factory::resource(): no plugin factory." << endl;
    return 0;
  }

  Resource *resource = pluginFactory->resource( config );
  if ( !resource ) {
    kdDebug(5650) << "'" << ptr->library() << "' is not a " + mResourceFamily +
                     " plugin." << endl;
    return 0;
  }

  resource->setType( type );

  return resource;
}
