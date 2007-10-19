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

static inline QString makeLibName( const QString &libname )
{
#ifdef Q_OS_WIN
    if (!libname.endsWith(".dll"))
        return libname + ".dll";
    return libname;
#else
    int pos = libname.lastIndexOf('/');
    if (pos < 0)
      pos = 0;
    if (libname.indexOf('.', pos) < 0) {
        const char* const extList[] = { ".so", ".dylib", ".bundle", ".sl" };
        for (uint i = 0; i < sizeof(extList) / sizeof(*extList); ++i) {
           if (QLibrary::isLibrary(libname + extList[i]))
               return libname + extList[i];
        }
    }
    return libname;
#endif
}

static inline QString findLibraryInternal(const QString &name, const KComponentData &cData)
{
    QString libname = makeLibName( name );

    // only look up the file if it is not an absolute filename
    // (mhk, 20000228)
    QString libfile;
    if (QDir::isRelativePath(libname)) {
      libfile = cData.dirs()->findResource("module", libname);
      if ( libfile.isEmpty() )
      {
        libfile = cData.dirs()->findResource("lib", libname);
#ifndef NDEBUG
        if ( !libfile.isEmpty() && libname.startsWith( "lib" ) ) // don't warn for kdeinit modules
          kDebug(150) << "library" << libname << "not found under 'module' but under 'lib'";
#endif
      }
    } else {
      libfile = libname;
    }
    return libfile;
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
          return libname;

      libname = findLibraryInternal(libname, cData);
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
        symname += QFileInfo(lib->fileName()).fileName().split(".").first().toLatin1();
    }

    const QString hashKey = lib->fileName() + QLatin1Char(':') + QString::fromAscii(symname);
    if (s_createdKde3Factories->contains(hashKey)) {
        KPluginFactory *factory = s_createdKde3Factories->value(hashKey);
        if (factory) {
            return factory;
        }
    }

    typedef KPluginFactory* (*t_func)();
    t_func func = reinterpret_cast<t_func>(lib->resolveFunction( symname ));
    if ( !func )
    {
        kDebug(150) << "The library" << lib->fileName() << "does not offer an"
                    << symname << "function.";
        return 0;
    }

    KPluginFactory* factory = func();

    if( !factory )
    {
        kDebug(150) << "The library" << lib->fileName() << "does not offer a KDE compatible factory.";
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
        kDebug(150) << "The library" << lib->fileName() << "does not offer a qt_plugin_instance function.";
        return 0;
    }

    QObject* instance = func();
    KPluginFactory *factory = qobject_cast<KPluginFactory *>(instance);

    if( !factory )
    {
        kDebug(150) << "The library" << lib->fileName() << "does not offer a KDE 4 compatible factory.";
        return 0;
    }
    return factory;
}

KPluginFactory* KLibrary::factory(const char* factoryname)
{
    KPluginFactory *factory = kde4Factory(this);
    if (!factory)
        factory = kde3Factory(this, factoryname);

    return factory;
}

void *KLibrary::resolveSymbol( const char* symname )
{
    void *sym = resolve( symname );

    return sym;
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
