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

#ifndef KRESOURCES_RESOURCEMANAGER_H
#define KRESOURCES_RESOURCEMANAGER_H

#include <qdict.h>
#include <qstringlist.h>

#include "resourcefactory.h"
#include "resourcemanagerimpl.h"

namespace KRES {

class Resource;

template<class T>
class ManagerListener
{
public:
  virtual void resourceAdded( T* resource ) = 0;
  virtual void resourceModified( T* resource ) = 0;
  virtual void resourceDeleted( T* resource ) = 0;
};

// TODO:
// The resource manager should provide some signals
// to warn applications that resources have been added,
// removed or modified.
//
// The manager should also keep track of which (or at least 
// how many) applications hve opened a resource, so that it
// is only closed if none of them is using it any more

template<class T>
class ResourceManager : private ManagerImplListener
{
public:
  ResourceManager<T>( const QString& family ) {
    factory = ResourceFactory::self( family );
    // The managerimpl will use the same Factory object as the manager
    // because of the ResourceFactory::self() pattern
    manager = new ResourceManagerImpl( family );
    manager->setListener( this );
    mListeners = new QPtrList<ManagerListener<T> >;
  }

  virtual ~ResourceManager<T>() { 
    manager->setListener( 0 );
    delete mListeners;
  }

  void sync() {
    manager->sync();
  }

  void add( T* resource ) {
    Resource* res = static_cast<Resource *>( resource );
    if ( res ) manager->add( res );
  }

  void remove( const T* resource ) {
    const Resource* res = static_cast<const Resource *>( resource );
    if ( res ) manager->remove( res );
  }

  T* standardResource() { return dynamic_cast<T *>( manager->standardResource() ); }

  void setStandardResource( const T* resource ) {
    const Resource* res = static_cast<const Resource *>( resource );
    if ( res ) manager->setStandardResource( res );
  }

  void setActive( T* resource, bool active ) {
    Resource* res = static_cast<Resource *>( resource );
    if ( res ) manager->setActive( res, active );
  }

  QPtrList<T> resources() { 
    QPtrList<Resource> list = manager->resources();
    QPtrList<T> result;
    Resource* res;
    for ( res = list.first(); res; res = list.next() ) {
      T* resource = dynamic_cast<T *>( res );
      if ( resource ) result.append( resource );
    }
    return result;
    // This only gives a warning, but it's too ugly.
//    return *reinterpret_cast<QPtrList<T> *>( &manager->resources() );
  }

  // Get only active or passive resources
  QPtrList<T> resources( bool active ) { 
    QPtrList<Resource> list = manager->resources( active );
    QPtrList<T> result;
    Resource* res;
    for ( res = list.first(); res; res = list.next() ) {
      T* resource = dynamic_cast<T *>( res );
      if ( resource ) result.append( resource );
    }
    return result;
//    return static_cast<QPtrList<T> > ( manager->resources( active ) );
  }

  /**
   * Returns a list of the names of the reources managed by the
   * ResourceManager for this family.
   */
  QStringList resourceNames() { return manager->resourceNames(); }


  ResourceConfigWidget *configWidget( const QString& type, QWidget *parent = 0 ) { return factory->resourceConfigWidget( type, parent ); }

  /**
   * Creates a new resource of type @param type, with default 
   * settings. The resource is 
   * not added to the manager, the application has to do that.
   * Returns a pointer to a resource object or a null pointer
   * if resource type doesn't exist.
   *
   * @param type   The type of the resource, one of those returned 
   *               by @ref resourceTypeNames()
   */
  T *createResource( const QString& type ) {
    return dynamic_cast<T *>( factory->resource( type, 0 ) );
  }

  /**
   * Returns a list of the names of all available resource types.
   */
  QStringList resourceTypeNames() { return factory->resourceTypeNames(); }

  void resourceChanged( const T* resource ) { 
    const Resource* res = static_cast<const Resource *>( resource );
    manager->resourceChanged( res ); 
  }

  void addListener( ManagerListener<T> * listener ) {
    mListeners->append( listener );
  }

  void removeListener( ManagerListener<T> * listener ) {
    mListeners->remove( listener );
  }

  virtual void resourceAdded( Resource* res ) {
    kdDebug() << "ResourceManager::resourceAdded " << res->resourceName() << endl;
    T* resource = dynamic_cast<T *>( res );
    ManagerListener<T> *listener;
    for ( listener = mListeners->first(); listener; listener = mListeners->next() )
      listener->resourceAdded( resource );
  }

  virtual void resourceModified( Resource* res ) {
    kdDebug() << "ResourceManager::resourceModified " << res->resourceName() << endl;
    T* resource = dynamic_cast<T *>( res );
    ManagerListener<T> *listener;
    for ( listener = mListeners->first(); listener; listener = mListeners->next() )
      listener->resourceModified( resource );
  }
  virtual void resourceDeleted( Resource* res ) {
    kdDebug() << "ResourceManager::resourceDeleted " << res->resourceName() << endl;
    T* resource = dynamic_cast<T *>( res );
    ManagerListener<T> *listener;
    for ( listener = mListeners->first(); listener; listener = mListeners->next() ) {
      kdDebug() << "Notifying a listener to ResourceManager..." << endl;
      listener->resourceDeleted( resource );
    }
  }


private:
  ResourceManagerImpl* manager;
  ResourceFactory* factory;
  QPtrList<ManagerListener<T> > *mListeners;
};

}

#endif
