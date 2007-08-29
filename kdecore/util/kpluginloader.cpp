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

#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include "kpluginfactory.h"
#include <kservice.h>
#include "klibloader.h"

#include <QtCore/QLibrary>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>


class KPluginLoaderPrivate
{
    Q_DECLARE_PUBLIC(KPluginLoader)
protected:
    KPluginLoaderPrivate(const QString &libname)
        : name(libname), pluginVersion(-1), verificationData(0), lib(0)
    {}

    KPluginLoader *q_ptr;
    const QString name;
    quint32 pluginVersion;
    KDEPluginVerificationData *verificationData;
    QString errorString;

    KLibrary *lib;
};

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
    QString libname = makeLibName(name);

    if (QFileInfo(name).fileName().startsWith("lib"))
        kDebug(150) << "plugins shouldn't have a 'lib' suffix:" << libname;

    QString libfile;
    if (QDir::isRelativePath(libname)) {
        libfile = cData.dirs()->findResource("module", libname);
        if ( libfile.isEmpty() )
        {
            libfile = cData.dirs()->findResource("lib", libname);
            if (!libfile.isEmpty())
                kDebug(150) << "library" << libname << "not found under 'module' but under 'lib'";
        }
    }
    else {
        libfile = libname;
    }
    return libfile;
}

bool KPluginLoader::isLoaded() const
{
    return QPluginLoader::isLoaded() || d_ptr->lib;
}

KPluginLoader::KPluginLoader(const QString &plugin, const KComponentData &componentdata, QObject *parent)
    : QPluginLoader(findLibraryInternal(plugin, componentdata), parent), d_ptr(new KPluginLoaderPrivate(plugin))
{
    d_ptr->q_ptr = this;
    load();
}

KPluginLoader::KPluginLoader(const KService &service, const KComponentData &componentdata, QObject *parent)
: QPluginLoader(findLibraryInternal(service.library(), componentdata), parent), d_ptr(new KPluginLoaderPrivate(service.library()))
{
    d_ptr->q_ptr = this;
    Q_D(KPluginLoader);
    Q_ASSERT(service.isValid());

    if (service.isApplication()) {
        d->errorString = i18n("The service provides no library, the Library key is missing in the .desktop file");
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

    KPluginFactory *factory;

    if (d->lib) {
        factory = d->lib->factory(d->name.toUtf8());
        if (factory == 0) {
            d->errorString = KLibLoader::self()->lastErrorMessage();
        }
        return factory;
    }

    QObject *obj = instance();

    if (!obj)
        return 0;

    factory = qobject_cast<KPluginFactory *>(obj);

    if (factory == 0) {
        delete obj;
        d->errorString = i18n("The library %1 does not offer a KDE 4 compatible factory." , d->name);
    }

    return factory;
}

bool KPluginLoader::load()
{
    Q_D(KPluginLoader);
    if (!QPluginLoader::load()) {
        d->lib = KLibLoader::self()->library(d->name);
        if (d->lib)
            return true;

        return false;
    }

    QLibrary lib(fileName());
    lib.load();
    Q_ASSERT(lib.isLoaded());

    d->verificationData = (KDEPluginVerificationData *) lib.resolve("kde_plugin_verification_data");
    if (d->verificationData) {
        if ((d->verificationData->KDEVersion > KDE_VERSION) || (KDE_VERSION_MAJOR << 16 != (d->verificationData->KDEVersion & 0xFF0000))) {
            d->errorString = i18n("The plugin '%1' uses incompatible KDE library. (%2)", d->name, d->verificationData->KDEVersionString);
            lib.unload();
            unload();
            return false;
        }
    } else {
        kDebug(150) << "The plugin" << d->name << "doesn't contain a kde_plugin_verification_data structure";
    }

    quint32 *version = (quint32 *) lib.resolve("kde_plugin_version");
    if (version)
        d->pluginVersion = *version;
    else
        d->pluginVersion = -1;

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
