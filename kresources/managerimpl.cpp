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

#include <dcopclient.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <krandom.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kstandarddirs.h>

#include "resource.h"
#include "factory.h"
#include "manager.h"
#include "managerimpl.h"
#include "manageriface_stub.h"

using namespace KRES;

ManagerImpl::ManagerImpl( ManagerNotifier *notifier, const QString &family )
  : DCOPObject( "ManagerIface_" + family.toUtf8() ),
    mNotifier( notifier ),
    mFamily( family ), mConfig( 0 ), mStdConfig( 0 ), mStandard( 0 ),
    mFactory( 0 ), mConfigRead( false )
{
  kDebug(5650) << "ManagerImpl::ManagerImpl()" << endl;

  mId = KRandom::randomString( 8 );

  // Register with DCOP
  if ( !KApplication::dcopClient()->isRegistered() ) {
    KApplication::dcopClient()->registerAs( "KResourcesManager" );
    KApplication::dcopClient()->setDefaultObject( objId() );
  }

  kDebug(5650) << "Connecting DCOP signals..." << endl;
  if ( !connectDCOPSignal( 0, "ManagerIface_" + family.toUtf8(),
                           "signalKResourceAdded( QString, QString )",
                           "dcopKResourceAdded( QString, QString )", false ) )
    kWarning(5650) << "Could not connect ResourceAdded signal!" << endl;

  if ( !connectDCOPSignal( 0, "ManagerIface_" + family.toUtf8(),
                           "signalKResourceModified( QString, QString )",
                           "dcopKResourceModified( QString, QString )", false ) )
    kWarning(5650) << "Could not connect ResourceModified signal!" << endl;

  if ( !connectDCOPSignal( 0, "ManagerIface_" + family.toUtf8(),
                           "signalKResourceDeleted( QString, QString )",
                           "dcopKResourceDeleted( QString, QString )", false ) )
    kWarning(5650) << "Could not connect ResourceDeleted signal!" << endl;

  KApplication::dcopClient()->setNotifications( true );
}

ManagerImpl::~ManagerImpl()
{
  kDebug(5650) << "ManagerImpl::~ManagerImpl()" << endl;

  Resource::List::ConstIterator it;
  for ( it = mResources.begin(); it != mResources.end(); ++it ) {
    delete *it;
  }
 
  delete mStdConfig;
}

void ManagerImpl::createStandardConfig()
{
  if ( !mStdConfig ) {
    QString file = defaultConfigFile( mFamily );
    mStdConfig = new KConfig( file );
  }
  
  mConfig = mStdConfig;
}

void ManagerImpl::readConfig( KConfig *cfg )
{
  kDebug(5650) << "ManagerImpl::readConfig()" << endl;

  delete mFactory;
  mFactory = Factory::self( mFamily );

  if ( !cfg ) {
    createStandardConfig();
  } else {
    mConfig = cfg;
  }

  mStandard = 0;

  mConfig->setGroup( "General" );

  QStringList keys = mConfig->readEntry( "ResourceKeys", QStringList() );
  keys += mConfig->readEntry( "PassiveResourceKeys", QStringList() );

  QString standardKey = mConfig->readEntry( "Standard" );

  for ( QStringList::Iterator it = keys.begin(); it != keys.end(); ++it ) {
    readResourceConfig( *it, false );
  }

  mConfigRead = true;
}

void ManagerImpl::writeConfig( KConfig *cfg )
{
  kDebug(5650) << "ManagerImpl::writeConfig()" << endl;

  if ( !cfg ) {
    createStandardConfig();
  } else {
    mConfig = cfg;
  }

  QStringList activeKeys;
  QStringList passiveKeys;

  // First write all keys, collect active and passive keys on the way
  Resource::List::Iterator it;
  for ( it = mResources.begin(); it != mResources.end(); ++it ) {
    writeResourceConfig( *it, false );

    QString key = (*it)->identifier();
    if( (*it)->isActive() )
      activeKeys.append( key );
    else
      passiveKeys.append( key );
  }

  // And then the general group

  kDebug(5650) << "Saving general info" << endl;
  mConfig->setGroup( "General" );
  mConfig->writeEntry( "ResourceKeys", activeKeys );
  mConfig->writeEntry( "PassiveResourceKeys", passiveKeys );
  if ( mStandard ) 
    mConfig->writeEntry( "Standard", mStandard->identifier() );
  else
    mConfig->writeEntry( "Standard", "" );

  mConfig->sync();
  kDebug(5650) << "ManagerImpl::save() finished" << endl;
}

void ManagerImpl::add( Resource *resource )
{
  resource->setActive( true );

  if ( mResources.isEmpty() ) {
    mStandard = resource;
  }

  mResources.append( resource );

  if ( mConfigRead )
    writeResourceConfig( resource, true );

  signalKResourceAdded( mId, resource->identifier() );
}

void ManagerImpl::remove( Resource *resource )
{
  if ( mStandard == resource ) mStandard = 0;
  removeResource( resource );

  mResources.removeAll( resource );

  signalKResourceDeleted( mId, resource->identifier() );

  delete resource;

  kDebug(5650) << "Finished ManagerImpl::remove()" << endl;
}

void ManagerImpl::change( Resource *resource )
{
  writeResourceConfig( resource, true );

  signalKResourceModified( mId, resource->identifier() );
}

void ManagerImpl::setActive( Resource *resource, bool active )
{
  if ( resource && resource->isActive() != active ) {
    resource->setActive( active );
  }
}

Resource *ManagerImpl::standardResource() 
{
  return mStandard;
}

void ManagerImpl::setStandardResource( Resource *resource ) 
{
  mStandard = resource;
}

// DCOP asynchronous functions

void ManagerImpl::dcopKResourceAdded( QString managerId, QString resourceId )
{
  if ( managerId == mId ) {
    kDebug(5650) << "Ignore DCOP notification to myself" << endl;
    return;
  }
  kDebug(5650) << "Receive DCOP call: added resource " << resourceId << endl;

  if ( getResource( resourceId ) ) {
    kDebug(5650) << "This resource is already known to me." << endl;
  }

  if ( !mConfig ) createStandardConfig();

  mConfig->reparseConfiguration();
  Resource *resource = readResourceConfig( resourceId, true );

  if ( resource ) {
    mNotifier->notifyResourceAdded( resource );
  } else 
    kError() << "Received DCOP: resource added for unknown resource "
              << resourceId << endl;
}

void ManagerImpl::dcopKResourceModified( QString managerId, QString resourceId )
{
  if ( managerId == mId ) {
    kDebug(5650) << "Ignore DCOP notification to myself" << endl;
    return;
  }
  kDebug(5650) << "Receive DCOP call: modified resource " << resourceId << endl;

  Resource *resource = getResource( resourceId );
  if ( resource ) {
    mNotifier->notifyResourceModified( resource );
  } else 
    kError() << "Received DCOP: resource modified for unknown resource "
              << resourceId << endl;
}

void ManagerImpl::dcopKResourceDeleted( QString managerId, QString resourceId )
{
  if ( managerId == mId ) {
    kDebug(5650) << "Ignore DCOP notification to myself" << endl;
    return;
  }
  kDebug(5650) << "Receive DCOP call: deleted resource " << resourceId << endl;

  Resource *resource = getResource( resourceId );
  if ( resource ) {
    mNotifier->notifyResourceDeleted( resource );

    kDebug(5650) << "Removing item from mResources" << endl;
    // Now delete item
    if ( mStandard == resource )
      mStandard = 0;
    mResources.removeAll( resource );
  } else
    kError() << "Received DCOP: resource deleted for unknown resource "
              << resourceId << endl;
}

QStringList ManagerImpl::resourceNames()
{
  QStringList result;

  Resource::List::ConstIterator it;
  for ( it = mResources.begin(); it != mResources.end(); ++it ) {
    result.append( (*it)->resourceName() );
  }
  return result;
}

Resource::List *ManagerImpl::resourceList()
{
  return &mResources;
}

QList<Resource *> ManagerImpl::resources()
{
  return QList<Resource *>( mResources );
}

QList<Resource *> ManagerImpl::resources( bool active )
{
  QList<Resource *> result;

  for(int i = 0; i < mResources.size(); ++i) {
    if( mResources.at(i)->isActive() == active) {
      result.append( mResources.at(i) );
    }
  }  
  return result;
}

Resource *ManagerImpl::readResourceConfig( const QString &identifier,
                                           bool checkActive )
{
  kDebug(5650) << "ManagerImpl::readResourceConfig() " << identifier << endl;

  if ( !mFactory ) {
    kError(5650) << "ManagerImpl::readResourceConfig: mFactory is 0. Did the app forget to call readConfig?" << endl;
    return 0;
  }

  mConfig->setGroup( "Resource_" + identifier );

  QString type = mConfig->readEntry( "ResourceType" );
  QString name = mConfig->readEntry( "ResourceName" );
  Resource *resource = mFactory->resource( type, mConfig );
  if ( !resource ) {
    kDebug(5650) << "Failed to create resource with id " << identifier << endl;
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
    QStringList activeKeys = mConfig->readEntry( "ResourceKeys", QStringList() );
    resource->setActive( activeKeys.contains( identifier ) );
  }
  mResources.append( resource );

  return resource;
}

void ManagerImpl::writeResourceConfig( Resource *resource, bool checkActive )
{
  QString key = resource->identifier();

  kDebug(5650) << "Saving resource " << key << endl;

  if ( !mConfig ) createStandardConfig();

  mConfig->setGroup( "Resource_" + key );
  resource->writeConfig( mConfig );

  mConfig->setGroup( "General" );
  QString standardKey = mConfig->readEntry( "Standard" );

  if ( resource == mStandard  && standardKey != key )
    mConfig->writeEntry( "Standard", resource->identifier() );
  else if ( resource != mStandard && standardKey == key )
    mConfig->writeEntry( "Standard", "" );
  
  if ( checkActive ) {
    QStringList activeKeys = mConfig->readEntry( "ResourceKeys", QStringList() );
    QStringList passiveKeys = mConfig->readEntry( "PassiveResourceKeys", QStringList() );
    if ( resource->isActive() ) {
      if ( passiveKeys.contains( key ) ) { // remove it from passive list
        passiveKeys.removeAll( key );
        mConfig->writeEntry( "PassiveResourceKeys", passiveKeys );
      }
      if ( !activeKeys.contains( key ) ) { // add it to active list
        activeKeys.append( key );
        mConfig->writeEntry( "ResourceKeys", activeKeys );
      }
    } else if ( !resource->isActive() ) {
      if ( activeKeys.contains( key ) ) { // remove it from active list
        activeKeys.removeAll( key );
        mConfig->writeEntry( "ResourceKeys", activeKeys );
      }
      if ( !passiveKeys.contains( key ) ) { // add it to passive list
        passiveKeys.append( key );
        mConfig->writeEntry( "PassiveResourceKeys", passiveKeys );
      }
    }
  }

  mConfig->sync();
}

void ManagerImpl::removeResource( Resource *resource )
{
  QString key = resource->identifier();

  if ( !mConfig ) createStandardConfig();
  
  mConfig->setGroup( "General" );
  QStringList activeKeys = mConfig->readEntry( "ResourceKeys", QStringList() );
  if ( activeKeys.contains( key ) ) {
    activeKeys.removeAll( key );
    mConfig->writeEntry( "ResourceKeys", activeKeys );
  } else {
    QStringList passiveKeys = mConfig->readEntry( "PassiveResourceKeys", QStringList() );
    passiveKeys.removeAll( key );
    mConfig->writeEntry( "PassiveResourceKeys", passiveKeys );
  }

  QString standardKey = mConfig->readEntry( "Standard" );
  if ( standardKey == key ) {
    mConfig->writeEntry( "Standard", "" );
  }

  mConfig->deleteGroup( "Resource_" + resource->identifier() );
  mConfig->sync();
}

Resource *ManagerImpl::getResource( const QString &identifier )
{
  Resource::List::ConstIterator it;
  for ( it = mResources.begin(); it != mResources.end(); ++it ) {
    if ( (*it)->identifier() == identifier )
      return *it;
  }
  return 0;
}

QString ManagerImpl::defaultConfigFile( const QString &family )
{
  return locateLocal( "config",
                      QString( "kresources/%1/stdrc" ).arg( family ) );
}
