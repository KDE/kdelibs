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

#ifndef KRESOURCES_RESOURCEMANAGER_IMPL_H
#define KRESOURCES_RESOURCEMANAGER_IMPL_H

#include <qstring.h>
#include <qptrlist.h>
#include <qdict.h>

#include "resourcemanageriface.h"

class KConfig;

namespace KRES {

class Resource;
class ResourceFactory;

class ManagerImplListener
{
  public:
    virtual void resourceAdded( Resource *resource ) = 0;
    virtual void resourceModified( Resource *resource ) = 0;
    virtual void resourceDeleted( Resource *resource ) = 0;
};


/**
  @internal

  Do not use this class directly. Use ResourceManager instead
*/
class ResourceManagerImpl : public QObject, virtual public ResourceManagerIface
{
    Q_OBJECT
  public:
    ResourceManagerImpl( const QString &family );
    ~ResourceManagerImpl();

    void sync();

    void add( Resource *resource, bool useDCOP = true );
    void remove( const Resource *resource, bool useDCOP = true );

    Resource *standardResource();
    void setStandardResource( const Resource *resource );

    void setActive( Resource *resource, bool active );

    QPtrList<Resource> resources();
    // Get only active or passive resources
    QPtrList<Resource> resources( bool active );

    QStringList resourceNames();

    void setListener( ManagerImplListener *listener )
    {
      mListener = listener;
    }

  public slots:
    void resourceChanged( const Resource *resource );

  private:
    // dcop calls
    void dcopResourceAdded( QString identifier );
    void dcopResourceModified( QString identifier );
    void dcopResourceDeleted( QString identifier );

  private:
    void load();
    void save();
    Resource *loadResource( const QString& identifier, bool checkActive, bool active=false );
    void saveResource( const Resource *item, bool checkActive );
    void removeResource( const Resource *item );
    Resource *getResource( const Resource *resource );
    Resource *getResource( const QString& identifier );

    const Resource *mStandard;
    QString mFamily;
    QPtrList<Resource> mResources;
    ManagerImplListener *mListener;
    bool mChanged;
    ResourceFactory *mFactory;
    KConfig *mConfig;
};

}

#endif
