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

#ifndef RESOURCEFACTORY_H
#define RESOURCEFACTORY_H

#include <qdict.h>

#include <kconfig.h>
#include <klibloader.h>

#include "resource.h"
#include "resourceconfigwidget.h"

namespace KABC {

struct ResourceInfo {
    QString library;
    QString name;
    QString desc;
};

class ResourceFactory
{
public:
    
    /**
     * Get the global resource factory.
     */
    static ResourceFactory *self();


    /**
     * Return the config widget for the
     * given resource name, or a null pointer
     * if resource doesn't exist.
     */
    ResourceConfigWidget *configWidget( const QString& resName, QWidget *parent = 0 );

    /**
     * Return a pointer to a resource object
     */
    Resource *resource( const QString& resName, AddressBook *ab, const KConfig *config );

    /**
     * Return a list of all available resources.
     */
    QStringList resources();

    /**
     * Return info to a resource.
     */
    ResourceInfo *info( const QString& resName );

protected:
    ResourceFactory();
    ~ResourceFactory();

private:
    KLibrary *openLibrary( const QString& libName );

    static ResourceFactory *mSelf;
    QDict<ResourceInfo> mResourceList;
};

}
#endif
