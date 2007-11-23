/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  Copyright (c) 2000 Matthias Elter <elter@kde.org>
  Copyright (c) 2003 Daniel Molkentin <molkentin@kde.org>
  Copyright (c) 2003,2006 Matthias Kretz <kretz@kde.org>

  This file is part of the KDE project

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2, as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "kcmoduleinfo.h"

#include <QtCore/QVariant>

#include <kdesktopfile.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>

class KCModuleInfo::Private
{
  public:
    Private();
    Private( KService::Ptr );

    QStringList keywords;
    QString     name, icon, lib, handle, fileName, doc, comment;
    bool        allLoaded;
    int         weight;

    KService::Ptr service;

    /**
     * Reads the service entries specific for KCModule from the desktop file.
     * The usual desktop entries are read in the Private ctor.
     */
    void loadAll();
};

KCModuleInfo::Private::Private()
{
}

KCModuleInfo::Private::Private( KService::Ptr s )
  : allLoaded( false )
  , service( s )
{
  if ( !service )
  {
    kDebug(712) << "Could not find the service.";
    return;
  }

  // set the modules simple attributes
  name = service->name();
  comment = service->comment();
  icon = service->icon();
  fileName = service->entryPath();
  lib = service->library();
  keywords = service->keywords();
}

KCModuleInfo::KCModuleInfo()
{
  d = new Private;
}

KCModuleInfo::KCModuleInfo(const QString& desktopFile)
{
  d = new Private( KService::serviceByStorageId(desktopFile) );
}

KCModuleInfo::KCModuleInfo( KService::Ptr moduleInfo )
{
  d = new Private( moduleInfo );
}

KCModuleInfo::KCModuleInfo( const KCModuleInfo &rhs )
{
    d = new Private;
    ( *this ) = rhs;
}

KCModuleInfo &KCModuleInfo::operator=( const KCModuleInfo &rhs )
{
    *d = *(rhs.d);
    return *this;
}

bool KCModuleInfo::operator==( const KCModuleInfo & rhs ) const
{
  return ( ( d->name == rhs.d->name ) && ( d->lib == rhs.d->lib ) && ( d->fileName == rhs.d->fileName ) );
}

bool KCModuleInfo::operator!=( const KCModuleInfo & rhs ) const
{
  return ! operator==( rhs );
}

KCModuleInfo::~KCModuleInfo()
{
  delete d;
}

void KCModuleInfo::Private::loadAll()
{
  allLoaded = true;

  if( !service ) /* We have a bogus service. All get functions will return empty/zero values */
    return;

  // get the documentation path
  doc = service->property( "X-DocPath", QVariant::String ).toString();
  if (doc.isEmpty())
    doc = service->property( "DocPath", QVariant::String ).toString();

  // read weight
  QVariant tmp = service->property( "X-KDE-Weight", QVariant::Int );
  weight = tmp.isValid() ? tmp.toInt() : 100;

  // factory handle
  tmp = service->property("X-KDE-FactoryName", QVariant::String);
  handle = tmp.isValid() ? tmp.toString() : lib;

}

QString KCModuleInfo::fileName() const
{
  return d->fileName;
}

QStringList KCModuleInfo::keywords() const
{
  return d->keywords;
}

QString KCModuleInfo::moduleName() const
{
  return d->name;
}

KService::Ptr KCModuleInfo::service() const
{
  return d->service;
}

QString KCModuleInfo::comment() const
{
  return d->comment;
}

QString KCModuleInfo::icon() const
{
  return d->icon;
}

QString KCModuleInfo::library() const
{
  return d->lib;
}

QString KCModuleInfo::docPath() const
{
  if (!d->allLoaded)
    d->loadAll();

  return d->doc;
}

QString KCModuleInfo::handle() const
{
  if (!d->allLoaded)
    d->loadAll();

  return d->handle;
}

int KCModuleInfo::weight() const
{
  if (!d->allLoaded)
    d->loadAll();

  return d->weight;
}

// vim: ts=2 sw=2 et
