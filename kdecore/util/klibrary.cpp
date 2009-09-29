/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000 Michael Matz <matz@kde.org>
   Copyright (C) 2007 Bernhard Loos <nhuh.put@web.de.org>

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
#include "klibrary.h"

#include <QtCore/QDir>
#include <QtCore/QPointer>

#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kpluginfactory.h>
#include <kdebug.h>

extern QString makeLibName( const QString &libname );
extern QString findLibraryInternal(const QString &name, const KComponentData &cData);

int kLibraryDebugArea() {
    static int s_area = KDebug::registerArea("kdecore (KLibrary)");
    return s_area;
}

//static
QString findLibrary(const QString &name, const KComponentData &cData)
{
    QString libname = findLibraryInternal(name, cData);
#ifdef Q_OS_WIN
    // we don't have 'lib' prefix on windows -> remove it and try again
    if( libname.isEmpty() )
    {
      libname = name;
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
          return file;

      libname = findLibraryInternal(libname, cData);
      if( libname.isEmpty() )
        libname = name;
    }
#endif
    return libname;
}


KLibrary::KLibrary(QObject *parent)
    : QLibrary(parent), d_ptr(0)
{
}

KLibrary::KLibrary(const QString &name, const KComponentData &cData, QObject *parent)
    : QLibrary(findLibrary(name, cData), parent), d_ptr(0)
{
}

KLibrary::KLibrary(const QString &name, int verNum, const KComponentData &cData, QObject *parent)
    : QLibrary(findLibrary(name, cData), verNum, parent), d_ptr(0)
{
}

KLibrary::~KLibrary()
{
}

typedef QHash<QString, QPointer<KPluginFactory> > FactoryHash;

K_GLOBAL_STATIC(FactoryHash, s_createdKde3Factories)

static KPluginFactory* kde3Factory(KLibrary *lib, const QByteArray &factoryname)
{
    QByteArray symname = "init_";
    if(!factoryname.isEmpty()) {
        symname += factoryname;
    } else {
        symname += QFileInfo(lib->fileName()).fileName().split('.').first().toLatin1();
    }

    const QString hashKey = lib->fileName() + QLatin1Char(':') + QString::fromAscii(symname);
    KPluginFactory *factory = s_createdKde3Factories->value(hashKey);
    if (factory) {
        return factory;
    }

    typedef KPluginFactory* (*t_func)();
    t_func func = reinterpret_cast<t_func>(lib->resolveFunction( symname ));
    if ( !func )
    {
#ifdef Q_OS_WIN
        // a backup for cases when developer has set lib prefix for a plugin name (she should not...)
        if (!factoryname.startsWith("lib"))
            return kde3Factory(lib, QByteArray("lib")+symname.mid(5 /*"init_"*/));
#endif
        kDebug(kLibraryDebugArea()) << "The library" << lib->fileName() << "does not offer an"
                    << symname << "function.";
        return 0;
    }

    factory = func();

    if( !factory )
    {
        kDebug(kLibraryDebugArea()) << "The library" << lib->fileName() << "does not offer a KDE compatible factory.";
        return 0;
    }
    s_createdKde3Factories->insert(hashKey, factory);

    return factory;
}

static KPluginFactory *kde4Factory(KLibrary *lib)
{
    const QByteArray symname("qt_plugin_instance");

    typedef QObject* (*t_func)();
    t_func func = reinterpret_cast<t_func>(lib->resolveFunction(symname));
    if ( !func )
    {
        kDebug(kLibraryDebugArea()) << "The library" << lib->fileName() << "does not offer a qt_plugin_instance function.";
        return 0;
    }

    QObject* instance = func();
    KPluginFactory *factory = qobject_cast<KPluginFactory *>(instance);

    if( !factory )
    {
        if (instance)
            kDebug(kLibraryDebugArea()) << "Expected a KPluginFactory, got a" << instance->metaObject()->className();
        kDebug(kLibraryDebugArea()) << "The library" << lib->fileName() << "does not offer a KDE 4 compatible factory.";
        return 0;
    }
    return factory;
}

// deprecated
KPluginFactory* KLibrary::factory(const char* factoryname)
{
    if (fileName().isEmpty()) {
        return NULL;
    }

    KPluginFactory *factory = kde4Factory(this);
    if (!factory)
        factory = kde3Factory(this, factoryname);

    return factory;
}

void *KLibrary::resolveSymbol( const char* symname )
{
    return resolve( symname );
}

KLibrary::void_function_ptr KLibrary::resolveFunction( const char* symname )
{
    void *psym = resolve( symname );
    if (!psym)
        return 0;

    // Cast the void* to non-pointer type first - it's not legal to
    // cast a pointer-to-object directly to a pointer-to-function.
    ptrdiff_t tmp = reinterpret_cast<ptrdiff_t>(psym);
    void_function_ptr sym = reinterpret_cast<void_function_ptr>(tmp);

    return sym;
}

void KLibrary::setFileName(const QString &name, const KComponentData &data)
{
    QLibrary::setFileName(findLibrary(name, data));
}

#include "klibrary.moc"
