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
  kdDebug(5650) << "ResourceManagerImpl::ResourceManagerImpl()" << endl;
  mConfig = 0;
  mStandard = 0;
  mFactory = 0;
  mChanged = false;

  mResources.setAutoDelete( true );

  load();

  // Register with DCOP
  if ( !kapp->dcopClient()->isRegistered() ) {
    kapp->dcopClient()->registerAs( "resourcemanager" );
    kapp->dcopClient()->setDefaultObject( objId() );
  }

  kdDebug(5650) << "Connecting DCOP signals..." << endl;
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
  kdDebug(5650) << "ResourceManagerImpl::~ResourceManagerImpl()" << endl;

  delete mConfig;
}

void ResourceManagerImpl::sync()
{
  kdDebug(5650) << "ResourceManagerImpl::sync()" << endl;
  if ( mChanged ) {
    save();
  }
}

void ResourceManagerImpl::add( Resource *resource, bool useDCOP )
{
  resource->setActive( true );

  if ( mResources.isEmpty() ) {
    mStandard = resource;
  }

  mResources.append( resource );
  mChanged = true;

  saveResource( resource, true );

  if ( useDCOP ) signalResourceAdded( resource->identifier() );
}

void ResourceManagerImpl::remove( const Resource *resource, bool useDCOP )
{
  if ( mStandard == resource ) mStandard = 0;
  removeResource( resource );

  mResources.remove( resource );
  mChanged = true;

  if ( useDCOP ) signalResourceDeleted( resource->identifier() );

  kdDebug(5650) << "Finished REsourceManagerImpl::remove()" << endl;
}

void ResourceManagerImpl::setActive( Resource *resource, bool active )
{
  if ( resource && resource->isActive() != active ) {
    resource->setActive( active );
    mChanged = true;
  }
}

Resource* ResourceManagerImpl::standardResource() 
{
  return mStandard;
}

void ResourceManagerImpl::setStandardResource( const Resource *resource ) 
{
  mStandard = resource;
}

void ResourceManagerImpl::resourceChanged( const Resource *resource )
{
  mChanged = true;

  saveResource( resource, true );

  signalResourceModified( resource->identifier() );
//  ResourceManagerIface_stub allManagers( "*", "ResourceManagerIface_" + mFamily.utf8() );
//  allManagers.dcopResourceModified( resource->identifier() );
}

// DCOP asynchronous functions

void ResourceManagerImpl::dcopResourceAdded( QString identifier )
{
  if ( kapp->dcopClient()->senderId() == kapp->dcopClient()->appId() ) {
    kdDebug(5650) << "Ignore DCOP call since am self source" << endl;
    return;
  }
  kdDebug(5650) << "Receive DCOP call: added resource " << identifier << endl;

  if ( getResource( identifier ) ) {
    kdDebug(5650) << "Wait a minute! This resource is already known to me!" << endl;
  }

  mConfig->reparseConfiguration();
  Resource *resource = loadResource( identifier, true );

  if ( resource ) {
    if ( mListener ) {
      kdDebug(5650) << "Notifying Listener" << endl;
      mListener->resourceAdded( resource );
    }
  }
  else 
    kdError() << "Received DCOP: resource added for unknown resource " << identifier << endl;
}

void ResourceManagerImpl::dcopResourceModified( QString identifier )
{
  if ( kapp->dcopClient()->senderId() == kapp->dcopClient()->appId() ) {
    kdDebug(5650) << "Ignore DCOP call since am self source" << endl;
    return;
  }
  kdDebug(5650) << "Receive DCOP call: modified resource " << identifier << endl;

  Resource *resource = getResource( identifier );
  if ( resource ) {
    if ( mListener ) {
      kdDebug(5650) << "Notifying Listener" << endl;
      mListener->resourceModified( resource );
    }
  } else 
    kdError() << "Received DCOP: resource modified for unknown resource " << identifier << endl;
}

void ResourceManagerImpl::dcopResourceDeleted( QString identifier )
{
  if ( kapp->dcopClient()->senderId() == kapp->dcopClient()->appId() ) {
    kdDebug(5650) << "Ignore DCOP call since am self source" << endl;
    return;
  }
  kdDebug(5650) << "Receive DCOP call: deleted resource " << identifier << endl;

  Resource *resource = getResource( identifier );
  if ( resource ) {
    if ( mListener ) {
      kdDebug(5650) << "Notifying Listener" << endl;
      mListener->resourceDeleted( resource );
    }

    kdDebug(5650) << "Removing item from mResources" << endl;
    // Now delete item
    if ( mStandard == resource )
      mStandard = 0;
    mResources.remove( resource );
  } else
    kdError() << "Received DCOP: resource deleted for unknown resource " << identifier << endl;

}

QStringList ResourceManagerImpl::resourceNames()
{
  QStringList result;

  Resource *item;
  for ( item = mResources.first(); item; item = mResources.next() ) {
    result.append( item->resourceName() );
  }
  return result;
}

QPtrList<Resource> ResourceManagerImpl::resources()
{
  QPtrList<Resource> result;

  Resource *item;
  for ( item = mResources.first(); item; item = mResources.next() ) {
    result.append( item );
  }
  return result;
}

QPtrList<Resource> ResourceManagerImpl::resources( bool active )
{
  QPtrList<Resource> result;

  Resource *item;
  for ( item = mResources.first(); item; item = mResources.next() ) {
    if ( item->isActive() == active ) {
      result.append( item );
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
  kdDebug(5650) << "ResourceManagerImpl::load()" << endl;

  delete mFactory;
  mFactory = ResourceFactory::self( mFamily );

  delete mConfig;
  mConfig = new KConfig( mFamily );

  mStandard = 0;

  mConfig->setGroup( "General" );

  QStringList keys = mConfig->readListEntry( "ResourceKeys" );
  uint numActiveKeys = keys.count();
  keys += mConfig->readListEntry( "PassiveResourceKeys" );

  QString standardKey = mConfig->readEntry( "Standard" );

  uint counter = 0;
//  bool haveStandardResource = false;
  for ( QStringList::Iterator it = keys.begin(); it != keys.end(); ++it ) {
    loadResource( *it, false, ( counter < numActiveKeys ) );
    counter++;
  }

  mChanged = false;
}


Resource* ResourceManagerImpl::loadResource( const QString& identifier, bool checkActive, bool active )
{
  if ( !mConfig ) mConfig = new KConfig( mFamily );
  mConfig->setGroup( "Resource_" + identifier );

  QString type = mConfig->readEntry( "ResourceType" );
  QString name = mConfig->readEntry( "ResourceName" );
  Resource *resource = mFactory->resource( type, mConfig );
  if ( !resource ) {
    kdDebug(5650) << "Failed to create resource with id " << identifier << endl;
    return 0;
  }

  if ( resource->identifier().isEmpty() )
    resource->setIdentifier( identifier );

  mConfig->setGroup( "General" );

  QString standardKey = mConfig->readEntry( "Standard" );
  if ( standardKey == identifier ) {
    mStandard = resource;
  }

  if ( checkActive ) {
    QStringList activeKeys = mConfig->readListEntry( "ResourceKeys" );
    resource->setActive( activeKeys.contains( identifier ) );
  }
  mResources.append( resource );

  return resource;
}

void ResourceManagerImpl::save()
{
  kdDebug(5650) << "ResourceManagerImpl::save()" << endl;
  QStringList activeKeys;
  QStringList passiveKeys;

  // First write all keys, collect active and passive keys on the way
  Resource *item;
  for ( item = mResources.first(); item; item = mResources.next() ) {
    saveResource( item, false );

    QString key = item->identifier();
    if( item->isActive() )
      activeKeys.append( key );
    else
      passiveKeys.append( key );
  }

  // And then the general group

  kdDebug(5650) << "Saving general info" << endl;
  mConfig->setGroup( "General" );
  mConfig->writeEntry( "ResourceKeys", activeKeys );
  mConfig->writeEntry( "PassiveResourceKeys", passiveKeys );
  if ( mStandard ) 
    mConfig->writeEntry( "Standard", mStandard->identifier() );
  else
    mConfig->writeEntry( "Standard", "" );

  mConfig->sync();
  mChanged = false;
  kdDebug(5650) << "ResourceManagerImpl::save() finished" << endl;
}

void ResourceManagerImpl::saveResource( const Resource *resource, bool checkActive )
{
  QString key = resource->identifier();

  kdDebug(5650) << "Saving resource " << key << endl;

  if ( !mConfig ) mConfig = new KConfig( mFamily );

  mConfig->setGroup( "Resource_" + key );
  resource->writeConfig( mConfig );

  mConfig->setGroup( "General" );
  QString standardKey = mConfig->readEntry( "Standard" );

  if ( resource == mStandard  && standardKey != key )
    mConfig->writeEntry( "Standard", resource->identifier() );
  else if ( resource != mStandard && standardKey == key )
    mConfig->writeEntry( "Standard", "" );
  
  if ( checkActive ) {
    QStringList activeKeys = mConfig->readListEntry( "ResourceKeys" );
    if ( resource->isActive() && !activeKeys.contains( key ) ) {
      activeKeys.append( resource->identifier() );
      mConfig->writeEntry( "ResourceKeys", activeKeys );
    } else if ( !resource->isActive() && activeKeys.contains( key ) ) {
      activeKeys.remove( key );
      mConfig->writeEntry( "ResourceKeys", activeKeys );
    }
  }

  mConfig->sync();
}

void ResourceManagerImpl::removeResource( const Resource *resource )
{
  QString key = resource->identifier();

  if ( !mConfig ) mConfig = new KConfig( mFamily );
  
  mConfig->setGroup( "General" );
  QStringList activeKeys = mConfig->readListEntry( "ResourceKeys" );
  if ( activeKeys.contains( key ) ) {
    activeKeys.remove( key );
    mConfig->writeEntry( "ResourceKeys", activeKeys );
  } else {
    QStringList passiveKeys = mConfig->readListEntry( "PassiveResourceKeys" );
    passiveKeys.remove( key );
    mConfig->writeEntry( "PassiveResourceKeys", passiveKeys );
  }

  QString standardKey = mConfig->readEntry( "Standard" );
  if ( standardKey == key ) {
    mConfig->writeEntry( "Standard", "" );
  }

  mConfig->deleteGroup( "Resource_" + resource->identifier() );
  mConfig->sync();
}

Resource* ResourceManagerImpl::getResource( const QString& identifier )
{
  Resource *item;
  for ( item = mResources.first(); item; item = mResources.next() ) {
    if ( item->identifier() == identifier )
      return item;
  }
  return 0;
}

#include "resourcemanagerimpl.moc"
