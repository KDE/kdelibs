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

#ifndef KRESOURCES_MANAGER_H
#define KRESOURCES_MANAGER_H

#include <qdict.h>
#include <qstringlist.h>

#include <kdebug.h>
#include <kresources/factory.h>
#include <kresources/managerimpl.h>

namespace KRES {

class Resource;

/**
  Observer class for Manager class. For getting notified about changes of
  Resources managed by a Manager object implement this Observer interface and
  add an object of this implementation to the Manager using addObserver(). The
  resourceAdded(), resourceModified() and resourceDeleted() functions of your
  implementation will be called whenever resources managed by the Manager object
  are added, modified or deleted.
*/
template<class T>
class ManagerObserver
{
  public:
    virtual void resourceAdded( T *resource ) = 0;
    virtual void resourceModified( T *resource ) = 0;
    virtual void resourceDeleted( T *resource ) = 0;
};

/**
  @internal
*/
class ManagerNotifier
{
  public:
    virtual void notifyResourceAdded( Resource *resource ) = 0;
    virtual void notifyResourceModified( Resource *resource ) = 0;
    virtual void notifyResourceDeleted( Resource *resource ) = 0;
};

/**
  This class provides a manager for resources of a specified family. It takes
  care of loading and saving resource configurations and provides access to the
  resources and their attributes. External changes in the resource configuration
  are notified by the ManagerObserver interface. If your application needs to be
  notified about resource configuration changes, you have to subclass
  ManagerObserver and add it to the Manager by addObserver().

  Since KDE 3.4 it's required to supply your application with a *.desktop
  file for each resource family you introduce. The ServiceType should be of
  KResources/Manager.
*/
template<class T>
class Manager : private ManagerNotifier
{
  public:
    /**
      Iterator for iterations over all resources managed by a manager.
    */
    class Iterator
    {
        friend class Manager;
      public:
        Iterator() {};
        Iterator( const Iterator &it ) { mIt = it.mIt; }

        T *operator*() { return static_cast<T *>( *mIt ); }
        Iterator &operator++() { mIt++; return *this; }
        Iterator &operator++( int ) { mIt++; return *this; }
        Iterator &operator--() { mIt--; return *this; }
        Iterator &operator--( int ) { mIt--; return *this; }
        bool operator==( const Iterator &it ) { return mIt == it.mIt; }
        bool operator!=( const Iterator &it ) { return mIt != it.mIt; }

      private:
        Resource::List::Iterator mIt;
    };

    /**
      Return Iterator on first resource. If there is no resource returns end().
    */
    Iterator begin()
    {
      Iterator it;
      it.mIt = mImpl->resourceList()->begin();
      return it;
    }

    /**
      Return Iterator indicating end of resource list.
    */
    Iterator end()
    {
      Iterator it;
      it.mIt = mImpl->resourceList()->end();
      return it;
    }

    /**
      Iterator for iterations over only active resources managed by a manager.
    */
    class ActiveIterator
    {
        friend class Manager;
      public:
        ActiveIterator() : mList( 0 ) {};
        ActiveIterator( const ActiveIterator &it )
        {
          mIt = it.mIt;
          mList = it.mList;
        }

        T *operator*() { return static_cast<T *>( *mIt ); }
        ActiveIterator &operator++()
        {
          do { mIt++; } while ( checkActive() );
          return *this;
        }
        ActiveIterator &operator++( int )
        {
          do { mIt++; } while ( checkActive() );
          return *this;
        }
        ActiveIterator &operator--()
        {
          do { mIt--; } while ( checkActive() );
          return *this;
        }
        ActiveIterator &operator--( int )
        {
          do { mIt--; } while ( checkActive() );
          return *this;
        }
        bool operator==( const ActiveIterator &it ) { return mIt == it.mIt; }
        bool operator!=( const ActiveIterator &it ) { return mIt != it.mIt; }

      private:
        /**
          Check if iterator needs to be advanced once more.
        */
        bool checkActive()
        {
          if ( !mList || mIt == mList->end() ) return false;
          return !(*mIt)->isActive();
        }

        Resource::List::Iterator mIt;
        Resource::List *mList;
    };

    /**
      Return Iterator on first active resource. If there is no active resource
      returns end().
    */
    ActiveIterator activeBegin()
    {
      ActiveIterator it;
      it.mIt = mImpl->resourceList()->begin();
      it.mList = mImpl->resourceList();
      if ( it.mIt != mImpl->resourceList()->end() ) {
        if ( !(*it)->isActive() ) it++;
      }
      return it;
    }

    /**
      Return Iterator indicating end of active resource list.
    */
    ActiveIterator activeEnd()
    {
      ActiveIterator it;
      it.mIt = mImpl->resourceList()->end();
      it.mList = mImpl->resourceList();
      return it;
    }

    /**
      Return true, if manager doesn't hold any resources. If there are resources
      return false.
    */
    bool isEmpty() const { return mImpl->resourceList()->isEmpty(); }

    /**
      Create manager for given resource family. The family argument is used as
      identifier for loading and saving resource configurations.
    */
    Manager( const QString &family )
    {
      mFactory = Factory::self( family );
      // The managerimpl will use the same Factory object as the manager
      // because of the Factory::self() pattern
      mImpl = new ManagerImpl( this, family );
      mObservers.setAutoDelete( false );
    }

    virtual ~Manager()
    {
      delete mImpl;
    }

    /**
      Recreate Resource objects from configuration file. If cfg is 0, read
      standard configuration file determined by family name.
    */
    void readConfig( KConfig *cfg = 0 )
    {
      mImpl->readConfig( cfg );
    }

    /**
      Write configuration of Resource objects to configuration file. If cfg is
      0, write to standard configuration file determined by family name.
    */
    void writeConfig( KConfig *cfg = 0 )
    {
      mImpl->writeConfig( cfg );
    }

    /**
      Add resource to manager. This passes ownership of the Resource object
      to the manager.
    */
    void add( Resource *resource )
    {
      if ( resource ) mImpl->add( resource );
    }

    /**
      Remove resource from manager. This deletes the Resource object.
    */
    void remove( Resource *resource )
    {
      if ( resource ) mImpl->remove( resource );
    }

    /**
      Call this to notify manager about changes of the configuration of the
      given resource.
    */
    void change( T *resource )
    {
      mImpl->change( resource );
    }

    /**
      Return standard resource.
    */
    T *standardResource()
    {
      return static_cast<T *>( mImpl->standardResource() );
    }

    /**
      Set standard resource.
    */
    void setStandardResource( T *resource )
    {
      if ( resource ) mImpl->setStandardResource( resource );
    }

    /**
      Set active state of resource.
    */
    void setActive( Resource *resource, bool active )
    {
      if ( resource ) mImpl->setActive( resource, active );
    }

    /**
      Returns a list of the names of the resources managed by the
      Manager for this family.
    */
    QStringList resourceNames() const
    {
      return mImpl->resourceNames();
    }

    /**
      Creates a new resource of type @p type with default
      settings. The resource is
      not added to the manager, the application has to do that.
      Returns a pointer to a resource object or a null pointer
      if resource type doesn't exist.

      @param type   The type of the resource, one of those returned
                    by resourceTypeNames()
    */
    T *createResource( const QString& type )
    {
      return dynamic_cast<T *>( mFactory->resource( type, 0 ) );
    }

    /**
      Returns a list of the names of all available resource types.
    */
    QStringList resourceTypeNames() const
    {
      return mFactory->typeNames();
    }

    /**
      Return list of descriptions of all available resource types.
    */
    QStringList resourceTypeDescriptions() const
    {
      QStringList typeDescs;
      QStringList types = mFactory->typeNames();

      for ( QStringList::ConstIterator it = types.begin(); it != types.end();
            ++it ) {
        QString desc = mFactory->typeName( *it );
        if ( !mFactory->typeDescription( *it ).isEmpty() )
          desc += " (" + mFactory->typeDescription( *it ) + ")";

        typeDescs.append( desc );
      }

      return typeDescs;
    }

    /**
      Add observer for resource changes to manager. See ManagerObserver. The
      Manager does not take ownership of the Observer object.
    */
    void addObserver( ManagerObserver<T> *observer )
    {
      mObservers.append( observer );
    }

    /**
      Remove Observer for resource changes from manager. See ManagerObserver.
      The Observer is not deleted by the Manager after being removed.
    */
    void removeObserver( ManagerObserver<T> *observer )
    {
      mObservers.remove( observer );
    }

  private:
    /**
      Implementation of the ManagerNotifier interface.
    */
    void notifyResourceAdded( Resource *res )
    {
      kdDebug(5650) << "Manager::resourceAdded " << res->resourceName() << endl;
      T *resource = dynamic_cast<T *>( res );
      if ( resource ) {
        ManagerObserver<T> *observer;
        for ( observer = mObservers.first(); observer;
              observer = mObservers.next() )
          observer->resourceAdded( resource );
      }
    }

    /**
      Implementation of the ManagerNotifier interface.
    */
    void notifyResourceModified( Resource *res )
    {
      kdDebug(5650) << "Manager::resourceModified " << res->resourceName()
                    << endl;
      T *resource = dynamic_cast<T *>( res );
      if ( resource ) {
        ManagerObserver<T> *observer;
        for ( observer = mObservers.first(); observer;
              observer = mObservers.next() )
          observer->resourceModified( resource );
      }
    }

    /**
      Implementation of the ManagerNotifier interface.
    */
    void notifyResourceDeleted( Resource *res )
    {
      kdDebug(5650) << "Manager::resourceDeleted " << res->resourceName()
                    << endl;
      T *resource = dynamic_cast<T *>( res );
      if ( resource ) {
        ManagerObserver<T> *observer;
        for ( observer = mObservers.first(); observer;
              observer = mObservers.next() ) {
          kdDebug(5650) << "Notifying a observer to Manager..." << endl;
          observer->resourceDeleted( resource );
        }
      }
    }

  private:
    ManagerImpl *mImpl;
    Factory *mFactory;
    QPtrList<ManagerObserver<T> > mObservers;
};

}

#endif
