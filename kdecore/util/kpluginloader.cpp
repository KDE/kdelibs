/*  This file is part of the KDE project
    Copyright (C) 2007 Bernhard Loos <nhuh.put@web.de>

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

#include "kpluginloader.h"

#include "kaboutdata.h"
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include "kpluginfactory.h"
#include <kservice.h>
#include "klibrary.h"
#include <kdebug.h>

#include <QtCore/QLibrary>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>

extern int kLibraryDebugArea();

class KPluginLoaderPrivate
{
    Q_DECLARE_PUBLIC(KPluginLoader)
protected:
    KPluginLoaderPrivate(const QString &libname)
        : name(libname), pluginVersion(~0U), verificationData(0), lib(0)
    {}
    ~KPluginLoaderPrivate()
    {
        delete lib;
    }

    KPluginLoader *q_ptr;
    const QString name;
    quint32 pluginVersion;
    KDEPluginVerificationData *verificationData;
    QString errorString;

    KLibrary *lib;
};

inline QString makeLibName( const QString &libname )
{
#if defined(Q_OS_WIN) || defined(Q_OS_CYGWIN)
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

#ifdef Q_OS_WIN
extern QString fixLibPrefix(const QString& libname);
#endif

QString findLibraryInternal(const QString &name, const KComponentData &cData)
{
    // Convert name to a valid platform libname
    QString libname = makeLibName(name);
    QFileInfo fileinfo(name);
    bool hasPrefix = fileinfo.fileName().startsWith("lib");
    bool kdeinit = fileinfo.fileName().startsWith("libkdeinit4_");

    if (hasPrefix && !kdeinit)
        kDebug(kLibraryDebugArea()) << "plugins should not have a 'lib' prefix:" << libname;
#ifdef Q_CC_MSVC
    // first remove the 'lib' prefix in front of windows plugins
    libname = fixLibPrefix(libname);
#endif

    // If it is a absolute path just return it
    if (!QDir::isRelativePath(libname))
        return libname;

    // Start looking
    QString libfile;

    // Check for kde modules/plugins?
    libfile = cData.dirs()->findResource("module", libname);
    if (!libfile.isEmpty())
        return libfile;

    // Now look where they don't belong but sometimes are
#ifndef Q_CC_MSVC
    if (!hasPrefix)
        libname = fileinfo.path() + QLatin1String("/lib") + fileinfo.fileName();
#endif

    libfile = cData.dirs()->findResource("lib", libname);
    if (!libfile.isEmpty()) {
        if (!kdeinit) {
            kDebug(kLibraryDebugArea()) << "library" << libname << "not found under 'module' but under 'lib'";
        }
        return libfile;
    }

    // Nothing found
    return QString();
}

bool KPluginLoader::isLoaded() const
{
    return QPluginLoader::isLoaded() || d_ptr->lib;
}

KPluginLoader::KPluginLoader(const QString &plugin, const KComponentData &componentdata, QObject *parent)
    : QPluginLoader(findLibraryInternal(plugin, componentdata), parent), d_ptr(new KPluginLoaderPrivate(plugin))
{
    d_ptr->q_ptr = this;
    Q_D(KPluginLoader);

    // No lib, no fun.
    if (fileName().isEmpty()) {
        d->errorString = i18n(
                "Could not find plugin '%1' for application '%2'",
                plugin,
                componentdata.aboutData()->appName());
        return;
    }

    load();
}


KPluginLoader::KPluginLoader(const KService &service, const KComponentData &componentdata, QObject *parent)
: QPluginLoader(findLibraryInternal(service.library(), componentdata), parent), d_ptr(new KPluginLoaderPrivate(service.library()))
{
    d_ptr->q_ptr = this;
    Q_D(KPluginLoader);

    // It's probably to late to check this because service.library() is used
    // above.
    if (!service.isValid()) {
        d->errorString = i18n("The provided service is not valid", service.entryPath());
        return;
    }

    // service.library() is used to find the lib. So first check if it is empty.
    if (service.library().isEmpty()) {
        d->errorString = i18n("The service '%1' provides no library or the Library key is missing in ", service.entryPath());
        return;
    }

    // No lib, no fun. service.library() was set but we were still unable to
    // find the lib.
    if (fileName().isEmpty()) {
        d->errorString = i18n(
                "Could not find plugin '%1' for application '%2'",
                service.name(),
                componentdata.aboutData()->appName());
        return;
    }

    load();
}

KPluginLoader::~KPluginLoader()
{
    delete d_ptr;
}

KPluginFactory *KPluginLoader::factory()
{
    Q_D(KPluginLoader);

    if (!isLoaded())
        return 0;


    if (d->lib) {
        // Calling a deprecated method, but this is the only way to
        // support both new and old-style factories for now.
        // KDE5: remove the whole if().
        return d->lib->factory(d->name.toUtf8());
    }

    QObject *obj = instance();

    if (!obj)
        return 0;

    KPluginFactory *factory = qobject_cast<KPluginFactory *>(obj);

    if (factory == 0) {
        kDebug(kLibraryDebugArea()) << "Expected a KPluginFactory, got a" << obj->metaObject()->className();
        delete obj;
        d->errorString = i18n("The library %1 does not offer a KDE 4 compatible factory." , d->name);
    }

    return factory;
}

bool KPluginLoader::load()
{
    Q_D(KPluginLoader);
    if (!QPluginLoader::load()) {
        d->lib = new KLibrary(d->name);
        if (d->lib->load())
            return true;

        return false;
    }

    Q_ASSERT(!fileName().isEmpty());
    QLibrary lib(fileName());
    lib.load();
    Q_ASSERT(lib.isLoaded());

    d->verificationData = (KDEPluginVerificationData *) lib.resolve("kde_plugin_verification_data");
    if (d->verificationData) {
        if (d->verificationData->dataVersion < KDEPluginVerificationData::PluginVerificationDataVersion
            || (d->verificationData->KDEVersion > KDE_VERSION)
            || (KDE_VERSION_MAJOR << 16 != (d->verificationData->KDEVersion & 0xFF0000)))
        {
            d->errorString = i18n("The plugin '%1' uses an incompatible KDE library (%2).", d->name, d->verificationData->KDEVersionString);
            lib.unload();
            unload();
            return false;
        }
    } else {
        kDebug(kLibraryDebugArea()) << "The plugin" << d->name << "doesn't contain a kde_plugin_verification_data structure";
    }

    quint32 *version = (quint32 *) lib.resolve("kde_plugin_version");
    if (version)
        d->pluginVersion = *version;
    else
        d->pluginVersion = ~0U;

    lib.unload();

    return true;
}

QString KPluginLoader::errorString() const
{
    Q_D(const KPluginLoader);
    if (!d->errorString.isEmpty())
        return d->errorString;

    return QPluginLoader::errorString();
}

quint32 KPluginLoader::pluginVersion() const
{
    Q_D(const KPluginLoader);
    return d->pluginVersion;
}

QString KPluginLoader::pluginName() const
{
    Q_D(const KPluginLoader);
    return d->name;
}

#include "kpluginloader.moc"
