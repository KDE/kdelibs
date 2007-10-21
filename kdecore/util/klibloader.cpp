/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000 Michael Matz <matz@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "klibloader.h"

#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QTimer>
#include <QtCore/QLibrary>
#include <QStack>
#include <QtCore/QCoreApplication>
#include <QtCore/QObjectCleanupHandler>

#include "kstandarddirs.h"
#include "kcomponentdata.h"
#include "kdebug.h"
#include "klocale.h"

class KLibLoaderPrivate
{
public:
    KLibLoader instance;
    QObjectCleanupHandler cleanuphandler;
    QString errorString;
};

K_GLOBAL_STATIC(KLibLoaderPrivate, kLibLoaderPrivate)

#define KLIBLOADER_PRIVATE KLibLoaderPrivate *const d = kLibLoaderPrivate

KLibLoader* KLibLoader::self()
{
    return &kLibLoaderPrivate->instance;
}

KLibLoader::KLibLoader( QObject* _parent )
    : QObject(_parent)
{
}

KLibLoader::~KLibLoader()
{
}

extern QString makeLibName( const QString &libname );

extern QString findLibraryInternal(const QString &name, const KComponentData &cData);

//static
QString KLibLoader::findLibrary(const QString &_name, const KComponentData &cData)
{
    QString libname = findLibraryInternal(QDir::fromNativeSeparators(_name), cData);
#ifdef Q_OS_WIN
    // we don't have 'lib' prefix on windows -> remove it and try again
    if( libname.isEmpty() )
    {
      libname = _name;
      QString file, path;

      int pos = libname.lastIndexOf( '/' );
      if ( pos >= 0 )
      {
        file = libname.mid( pos + 1 );
        path = libname.left( pos );
        libname = path + '/' + file.mid( 3 );
      }
      else
      {
        file = libname;
        libname = file.mid( 3 );
      }
      if( !file.startsWith( "lib" ) )
          return libname;

      libname = findLibraryInternal(libname, cData);
    }
#endif
    return libname;
}


KLibrary* KLibLoader::library( const QString &_name, QLibrary::LoadHints hint )
{
    if (_name.isEmpty())
        return 0;

    KLibrary *lib = new KLibrary(_name);

    // Klibrary search magic did work?
    if (lib->fileName().isEmpty()) {
        kLibLoaderPrivate->errorString = i18n("Library not found");
        return 0;
    }

    lib->setLoadHints(hint);

    lib->load();

    if (!lib->isLoaded()) {
        kLibLoaderPrivate->errorString = lib->errorString();
        delete lib;
        return 0;
    }

    kLibLoaderPrivate->cleanuphandler.add(lib);

    return lib;
}

QString KLibLoader::lastErrorMessage() const
{
    return kLibLoaderPrivate->errorString;
}

void KLibLoader::unloadLibrary( const QString &)
{
}

KPluginFactory* KLibLoader::factory( const QString &_name, QLibrary::LoadHints hint )
{
    KLibrary* lib = library( _name, hint);
    if ( !lib )
        return 0;

    KPluginFactory* fac = lib->factory();
    if ( !fac ) {
        kLibLoaderPrivate->errorString = errorString( ErrNoFactory );
        return 0;
    }

    return fac;
}

QString KLibLoader::errorString( int componentLoadingError )
{
    switch ( componentLoadingError ) {
    case ErrNoServiceFound:
        return i18n( "No service matching the requirements was found" );
    case ErrServiceProvidesNoLibrary:
        return i18n( "The service provides no library, the Library key is missing in the .desktop file" );
    case ErrNoLibrary:
        return KLibLoader::self()->lastErrorMessage();
    case ErrNoFactory:
        return i18n( "The library does not export a factory for creating components" );
    case ErrNoComponent:
        return i18n( "The factory does not support creating components of the specified type" );
    default:
        return i18n( "KLibLoader: Unknown error" );
    }
}

#include "klibloader.moc"
// vim: sw=4 sts=4 et
