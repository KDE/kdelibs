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

#include <kapplication.h>
#include <kdebug.h>
#include <kconfig.h>
#include <dcopclient.h>

#include "resource.h"
#include "resourcefactory.h"
#include "resourcemanagerimpl.h"
#include "resourcemanageriface_stub.h"

using namespace KRES;

ResourceManagerImpl::ResourceManagerImpl( const QString& family ) :
  DCOPObject( "ResourceManagerIface_" + family.utf8()+"_"+QCString().setNum(kapp->random()) ),
  mFamily( family )
{
  kdDebug(5700) << "ResourceManagerImpl::ResourceManagerImpl()" << endl;
  config = 0;
  mStandard = 0;
  mResources = 0;
  mFactory = 0;
  mChanged = false;
  load();

  // Register with DCOP
  if ( !kapp->dcopClient()->isRegistered() ) {
    kapp->dcopClient()->registerAs( "resourcemanager" );
    kapp->dcopClient()->setDefaultObject( objId() );
  }

  kdDebug() << "Connecting DCOP signals..." << endl;
  if ( ! connectDCOPSignal( 0, "ResourceManagerIface_" + family.utf8() + "_*", "signalResourceAdded(QString)", "dcopResourceAdded(QString)", false ) )
    kdWarning() << "Could not connect ResourceAdded signal!" << endl;

  // "ResourceManagerIface_" + mFamily.utf8()
  if ( ! connectDCOPSignal( 0, "ResourceManagerIface_" + family.utf8() + "_*", "signalResourceModified(QString)", "dcopResourceModified(QString)", false ) )
    kdWarning() << "Could not connect ResourceModified signal!" << endl;

  if ( ! connectDCOPSignal( 0, "ResourceManagerIface_" + family.utf8() + "_*", "signalResourceDeleted(QString)", "dcopResourceDeleted(QString)", false ) )
    kdWarning() << "Could not connect ResourceDeleted signal!" << endl;

  kapp->dcopClient()->setNotifications( true );

}

ResourceManagerImpl::~ResourceManagerImpl()
{
  kdDebug(5700) << "ResourceManagerImpl::~ResourceManagerImpl()" << endl;
  delete mResources;
  delete config;
}

void ResourceManagerImpl::sync()
{
  kdDebug() << "ResourceManagerImpl::sync()" << endl;
  if ( mChanged ) {
    save();
  }
}

void ResourceManagerImpl::add( Resource* resource, bool useDCOP )
{
  ResourceItem* item = new ResourceItem;
//  item->key = resource->identifier();
  item->resource = resource;
  item->active = true;
  if ( mResources->isEmpty() ) {
    item->standard = true;
    mStandard = item;
  } else {
    item->standard = false;
  }

  mResources->append( item );
  mChanged = true;

  saveResource( item, true );

  if ( useDCOP ) signalResourceAdded( resource->identifier() );
}

void ResourceManagerImpl::remove( const Resource* resource, bool useDCOP )
{
  ResourceItem *item = getItem( resource );
  if ( item ) {
    removeResource( item );

    if ( item->standard )
      mStandard = 0;
    mResources->remove( item );
//    delete item; Done automatically by the above line
    mChanged = true;

    if ( useDCOP ) signalResourceDeleted( resource->identifier() );
  } else
    kdDebug( 5700 ) << "ERROR: ResourceManagerImpl::remove(): Unknown resource" << endl;
  kdDebug() << "Finished REsourceManagerImpl::remove()" << endl;
}

void ResourceManagerImpl::setActive( Resource* resource, bool active )
{
  ResourceItem *item = getItem( resource );
  if ( item && item->active != active ) {
    item->active = active;
    mChanged = true;
  }
}

Resource* ResourceManagerImpl::standardResource() 
{
  if ( mStandard ) 
    return mStandard->resource; 
  else
    return 0;
}

void ResourceManagerImpl::setStandardResource( const Resource* resource ) 
{
  if ( mStandard )
    mStandard->standard = false;
  ResourceItem *item = getItem( resource );
  if ( item ) {
    mStandard = item;
    item->standard = true;
    mChanged = true;
  } else
    kdDebug( 5700 ) << "ERROR: ResourceManagerImpl::setStandardResource(): Unknown resource" << endl;
}

void ResourceManagerImpl::resourceChanged( const Resource* resource )
{
  mChanged = true;

  saveResource( getItem( resource ), true );

  signalResourceModified( resource->identifier() );
//  ResourceManagerIface_stub allManagers( "*", "ResourceManagerIface_" + mFamily.utf8() );
//  allManagers.dcopResourceModified( resource->identifier() );
}

// DCOP asynchronous functions

void ResourceManagerImpl::dcopResourceAdded( QString identifier )
{
  if ( kapp->dcopClient()->senderId() == kapp->dcopClient()->appId() ) {
    kdDebug() << "Ignore DCOP call since am self source" << endl;
    return;
  }
  kdDebug() << "Receive DCOP call: added resource " << identifier << endl;

  if ( getItem( identifier ) ) {
    kdDebug() << "Wait a minute! This resource is already known to me!" << endl;
  }

  config->reparseConfiguration();
  ResourceItem* item = loadResource( identifier, true );

  if ( item ) {
    if ( mListener ) {
      kdDebug() << "Notifying Listener" << endl;
      mListener->resourceAdded( item->resource );
    }
  }
  else 
    kdError() << "Received DCOP: resource added for unknown resource " << identifier << endl;
}

void ResourceManagerImpl::dcopResourceModified( QString identifier )
{
  if ( kapp->dcopClient()->senderId() == kapp->dcopClient()->appId() ) {
    kdDebug() << "Ignore DCOP call since am self source" << endl;
    return;
  }
  kdDebug() << "Receive DCOP call: modified resource " << identifier << endl;

  ResourceItem* item = getItem( identifier );
  if ( item ) {
    if ( mListener ) {
      kdDebug() << "Notifying Listener" << endl;
      mListener->resourceModified( item->resource );
    }
  } else 
    kdError() << "Received DCOP: resource modified for unknown resource " << identifier << endl;
}

void ResourceManagerImpl::dcopResourceDeleted( QString identifier )
{
  if ( kapp->dcopClient()->senderId() == kapp->dcopClient()->appId() ) {
    kdDebug() << "Ignore DCOP call since am self source" << endl;
    return;
  }
  kdDebug() << "Receive DCOP call: deleted resource " << identifier << endl;

  ResourceItem* item = getItem( identifier );
  if ( item ) {
    if ( mListener ) {
      kdDebug() << "Notifying Listener" << endl;
      mListener->resourceDeleted( item->resource );
    }

    kdDebug() << "Removing item from mResources" << endl;
    // Now delete item
    if ( item->standard )
      mStandard = 0;
    mResources->remove( item );
  } else
    kdError() << "Received DCOP: resource deleted for unknown resource " << identifier << endl;

}

QStringList ResourceManagerImpl::resourceNames()
{
  QStringList result;

  ResourceItem *item;
  for ( item = mResources->first(); item; item = mResources->next() ) {
    result.append( item->resource->resourceName() );
  }
  return result;
}

QPtrList<Resource> ResourceManagerImpl::resources()
{
  QPtrList<Resource> result;

  ResourceItem *item;
  for ( item = mResources->first(); item; item = mResources->next() ) {
    result.append( item->resource );
  }
  return result;
}

QPtrList<Resource> ResourceManagerImpl::resources( bool active )
{
  QPtrList<Resource> result;

  ResourceItem *item;
  for ( item = mResources->first(); item; item = mResources->next() ) {
    if ( item->active == active ) {
      result.append( item->resource );
    }
  }
  return result;
}


/*
void ResourceManagerImpl::defaults()
{
  // mConfigPage->defaults();
}
*/

//////////////////////////////
/// Config file handling /////
//////////////////////////////

void ResourceManagerImpl::load()
{
  kdDebug(5700) << "ResourceManagerImpl::load()" << endl;

  delete mFactory;
  mFactory = ResourceFactory::self( mFamily );

  delete config;
  config = new KConfig( mFamily );

  delete mResources;
  mResources = new QPtrList<ResourceItem>;
  mResources->setAutoDelete( true );

  mStandard = 0;

  config->setGroup( "General" );

  QStringList keys = config->readListEntry( "ResourceKeys" );
  uint numActiveKeys = keys.count();
  keys += config->readListEntry( "PassiveResourceKeys" );

  QString standardKey = config->readEntry( "Standard" );

  uint counter = 0;
//  bool haveStandardResource = false;
  for ( QStringList::Iterator it = keys.begin(); it != keys.end(); ++it ) {
    loadResource( *it, false, ( counter<numActiveKeys ) );
    counter++;
  }

  mChanged = false;
}


ResourceItem* ResourceManagerImpl::loadResource( const QString& identifier, bool checkActive, bool active )
{
  if ( ! config ) config = new KConfig( mFamily );
  config->setGroup( "Resource_" + identifier );

  QString type = config->readEntry( "ResourceType" );
  QString name = config->readEntry( "ResourceName" );
  Resource* resource = mFactory->resource( type, config );
  if ( ! resource ) {
    kdDebug() << "Failed to create resource with id " << identifier << endl;
    return 0;
  }

  if ( resource->identifier().isEmpty() )
    resource->setIdentifier( identifier );

  ResourceItem* item = new ResourceItem;
  item->resource = resource;

  config->setGroup( "General" );

  QString standardKey = config->readEntry( "Standard" );
  if ( standardKey == identifier ) {
    item->standard = true;
    mStandard = item;
  } else
    item->standard = false;

  if ( checkActive ) {
    QStringList activeKeys = config->readListEntry( "ResourceKeys" );
    item->active = ( activeKeys.contains( identifier ) );
  } else {
    item->active = active;
  }
  mResources->append( item );

  return item;
}

void ResourceManagerImpl::save()
{
  kdDebug() << "ResourceManagerImpl::save()" << endl;
  QStringList activeKeys;
  QStringList passiveKeys;

  // First write all keys, collect active and passive keys on the way
  ResourceItem *item;
  for ( item = mResources->first(); item; item = mResources->next() ) {
    saveResource( item, false );

    QString key = item->resource->identifier();
    if( item->active )
      activeKeys.append( key );
    else
      passiveKeys.append( key );
  }

  // And then the general group

  kdDebug() << "Saving general info" << endl;
  config->setGroup( "General" );
  config->writeEntry( "ResourceKeys", activeKeys );
  config->writeEntry( "PassiveResourceKeys", passiveKeys );
  if ( mStandard ) 
    config->writeEntry( "Standard", mStandard->resource->identifier() );
  else
    config->writeEntry( "Standard", "" );

  config->sync();
  mChanged = false;
  kdDebug() << "ResourceManagerImpl::save() finished" << endl;
}

void ResourceManagerImpl::saveResource( const ResourceItem* item, bool checkActive )
{
  Resource* resource = item->resource;
  QString key = resource->identifier();

  kdDebug() << "Saving resource " << key << endl;

  if ( ! config ) config = new KConfig( mFamily );

  config->setGroup( "Resource_" + key );
  resource->writeConfig( config );

  config->setGroup( "General" );
  QString standardKey = config->readEntry( "Standard" );

  if ( item->standard  && standardKey != key )
    config->writeEntry( "Standard", resource->identifier() );
  else if ( !item->standard && standardKey == key )
    config->writeEntry( "Standard", "" );
  
  if ( checkActive ) {
    QStringList activeKeys = config->readListEntry( "ResourceKeys" );
    if ( item->active && !activeKeys.contains( key ) ) {
      activeKeys.append( resource->identifier() );
      config->writeEntry( "ResourceKeys", activeKeys );
    } else if ( !item->active && activeKeys.contains( key ) ) {
      activeKeys.remove( key );
      config->writeEntry( "ResourceKeys", activeKeys );
    }
  }

  config->sync();
}

void ResourceManagerImpl::removeResource( const ResourceItem* item )
{
  Resource* resource = item->resource;
  QString key = resource->identifier();

  if ( ! config ) config = new KConfig( mFamily );
  
  config->setGroup( "General" );
  QStringList activeKeys = config->readListEntry( "ResourceKeys" );
  if ( activeKeys.contains( key ) ) {
    activeKeys.remove( key );
    config->writeEntry( "ResourceKeys", activeKeys );
  } else {
    QStringList passiveKeys = config->readListEntry( "PassiveResourceKeys" );
    passiveKeys.remove( key );
    config->writeEntry( "PassiveResourceKeys", passiveKeys );
  }

  QString standardKey = config->readEntry( "Standard" );
  if ( standardKey == key ) {
    config->writeEntry( "Standard", "" );
  }

  config->deleteGroup( "Resource_" + resource->identifier() );
  config->sync();
}

/////////////////////////////////////////////

ResourceItem* ResourceManagerImpl::getItem( const Resource* resource )
{
  ResourceItem *item;
  for ( item = mResources->first(); item; item = mResources->next() ) {
    if ( item->resource == resource )
      return item;
  }
  return 0;
}

ResourceItem* ResourceManagerImpl::getItem( const QString& identifier )
{
  ResourceItem *item;
  for ( item = mResources->first(); item; item = mResources->next() ) {
    if ( item->resource->identifier() == identifier )
      return item;
  }
  return 0;
}

#include "resourcemanagerimpl.moc"
