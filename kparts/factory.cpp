/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
             (C) 1999 David Faure <faure@kde.org>

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

#include "factory.h"
#include "part.h"

#include <QtGui/QWidget>

#include <klocale.h>
#include <kglobal.h>
#include <kcomponentdata.h>
#include <assert.h>

using namespace KParts;

Factory::Factory( QObject *parent )
: KLibFactory( parent )
{
}

Factory::~Factory()
{
}

Part *Factory::createPart( QWidget *parentWidget, QObject *parent, const char *classname, const QStringList &args )
{
    Part* part = createPartObject( parentWidget, parent, classname, args );
    if ( part )
	emit objectCreated( part );
    return part;
}

KComponentData Factory::partComponentData()
{
    return KComponentData();
}

KComponentData Factory::partComponentDataFromLibrary( const QString &libraryName )
{
    KLibrary *library = KLibLoader::self()->library( libraryName );
    if ( !library )
        return KComponentData();
    KLibFactory *factory = library->factory();
    if ( !factory )
        return KComponentData();
    KParts::Factory *pfactory = dynamic_cast<KParts::Factory *>( factory );
    if ( !pfactory )
        return KComponentData();
    return pfactory->partComponentData();
}

Part *Factory::createPartObject( QWidget *, QObject *, const char *, const QStringList & )
{
    return 0;
}

QObject *Factory::createObject( QObject *parent, const char *classname, const QStringList &args )
{
  assert( !parent || parent->isWidgetType() );
  return createPart( static_cast<QWidget *>( parent ), parent, classname, args );
}

#include "factory.moc"
