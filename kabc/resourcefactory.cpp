/*
    This file is part of libkabc.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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

#include <kdebug.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

#include <qfile.h>

#include "resourcefile.h"
#include "resourcefileconfig.h"

#include "resourcefactory.h"

using namespace KABC;

ResourceFactory *ResourceFactory::mSelf = 0;

ResourceFactory *ResourceFactory::self()
{
  kdDebug(5700) << "ResourceFactory::self()" << endl;

  if ( !mSelf ) {
    mSelf = new ResourceFactory;
  }

  return mSelf;
}

ResourceFactory::ResourceFactory()
{
  mResourceList.setAutoDelete( true );

  // dummy entry for default resource
  ResourceInfo *info = new ResourceInfo;
  mResourceList.insert( "file", info );

  QStringList list = KGlobal::dirs()->findAllResources( "data" ,"kabc/plugins/*.desktop", true, true );
  for ( QStringList::iterator it = list.begin(); it != list.end(); ++it ) {
    KSimpleConfig config( *it, true );

    if ( !config.hasGroup( "Misc" ) || !config.hasGroup( "Plugin" ) )
      continue;

    info = new ResourceInfo;

    config.setGroup( "Plugin" );
    QString type = config.readEntry( "Type" );
    info->library = config.readEntry( "X-KDE-Library" );
	
    config.setGroup( "Misc" );
    info->nameLabel = config.readEntry( "Name" );
    info->descriptionLabel = config.readEntry( "Comment", i18n( "No description available." ) );

    mResourceList.insert( type, info );
  }
}

ResourceFactory::~ResourceFactory()
{
  mResourceList.clear();
}

QStringList ResourceFactory::resources()
{
  QStringList retval;
	
  // make sure 'file' is the first entry
  retval << "file";

  QDictIterator<ResourceInfo> it( mResourceList );
  for ( ; it.current(); ++it )
    if ( it.currentKey() != "file" )
      retval << it.currentKey();

  return retval;
}

ResourceConfigWidget *ResourceFactory::configWidget( const QString& type, QWidget *parent )
{
  ResourceConfigWidget *widget = 0;

  if ( type.isEmpty() )
    return 0;

  if ( type == "file" )
    return new ResourceFileConfig( parent, "ResourceFileConfig" );

  QString libName = mResourceList[ type ]->library;

  KLibrary *library = openLibrary( libName );
  if ( !library )
    return 0;

  void *widget_func = library->symbol( "config_widget" );

  if ( widget_func ) {
    widget = ((ResourceConfigWidget* (*)(QWidget *wdg))widget_func)( parent );
  } else {
    kdDebug( 5700 ) << "'" << libName << "' is not a kabc plugin." << endl;
    return 0;
  }

  return widget;
}

ResourceInfo *ResourceFactory::info( const QString &type )
{
  if ( type.isEmpty() )
    return 0;
  else
    return mResourceList[ type ];
}

Resource *ResourceFactory::resource( const QString& type, AddressBook *ab, const KConfig *config )
{
  Resource *resource = 0;

  if ( type.isEmpty() )
    return 0;

  if ( type == "file" ) {
    resource = new ResourceFile( ab, config );
    resource->setType( type );
    resource->setNameLabel( i18n( "File" ) );
    resource->setDescriptionLabel( i18n( "File Resource" ) );
    return resource;
  }

  QString libName = mResourceList[ type ]->library;

  KLibrary *library = openLibrary( libName );
  if ( !library )
    return 0;

  void *resource_func = library->symbol( "resource" );

  if ( resource_func ) {
    resource = ((Resource* (*)(AddressBook *, const KConfig *))resource_func)( ab, config );
    resource->setType( type );
    resource->setNameLabel( mResourceList[ type ]->nameLabel );
    resource->setDescriptionLabel( mResourceList[ type ]->descriptionLabel );
  } else {
    kdDebug( 5700 ) << "'" << libName << "' is not a kabc plugin." << endl;
    return 0;
  }

  return resource;
}

KLibrary *ResourceFactory::openLibrary( const QString& libName )
{
  KLibrary *library = 0;

  QString path = KLibLoader::findLibrary( QFile::encodeName( libName ) );

  if ( path.isEmpty() ) {
    kdDebug( 5700 ) << "No resource plugin library was found!" << endl;
    return 0;
  }

  library = KLibLoader::self()->library( QFile::encodeName( path ) );

  if ( !library ) {
    kdDebug( 5700 ) << "Could not load library '" << libName << "'" << endl;
    return 0;
  }

  return library;
}
