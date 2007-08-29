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
#ifndef KDECORE_KPLUGINLOADER_H
#define KDECORE_KPLUGINLOADER_H

#include <kglobal.h>
#include <kdeversion.h>

#include <QtCore/QPluginLoader>
#include <QtCore/QtPlugin>

/**
 * \internal
 * Stores KDE version informations in a plugin library.
 * \see K_PLUGIN_VERIFICATION_DATA
 */
struct KDEPluginVerificationData
{
    /**
     * \internal
     * The version of this structure. Increase it in the K_EXPORT_PLUGIN_VERSION macro whenever you
     * make changes to this structure.
     */
    quint8 dataVersion;
    quint32 KDEVersion;
    const char *KDEVersionString;
};

/**
 * \internal
 * Used to export the KDE version a plugin was compiled against.
 * \see KDEPluginVerificationData
 */
#define K_PLUGIN_VERIFICATION_DATA \
Q_EXTERN_C KDE_EXPORT const KDEPluginVerificationData kde_plugin_verification_data = \
{ 1, KDE_VERSION, KDE_VERSION_STRING };

/**
 * \relates KPluginLoader
 * Use this macro if you want to give your plugin a version number.
 * You can later access the version number with KPluginLoader::pluginVersion()
 */
#define K_EXPORT_PLUGIN_VERSION(version) \
Q_EXTERN_C KDE_EXPORT const quint32 kde_plugin_version = version;


/**
 * \relates KPluginLoader
 * This macro exports the main object of the plugin. Most times, this will be a KPluginFactory 
 * or derived class, but any QObject derived class can be used.
 * Take a look at the documentation of Q_EXPORT_PLUGIN2 for some details.
 */
#define K_EXPORT_PLUGIN(factory) \
Q_EXPORT_PLUGIN(factory) \
K_PLUGIN_VERIFICATION_DATA

class KComponentData;
class KPluginFactory;
class KService;

class KPluginLoaderPrivate;

/**
 * This class can be used to dynamically load a plugin library at runtime.
 *
 * This class makes sure that the Qt and KDE versions used to compile this library aren't newer then
 * the ones currently used.
 *
 *
 * This class is reentrant, you can load plugins from different threads. You can also have multiple
 * PluginLoaders for one library without negative effects.
 * The object obtained with factory() or the inherited methode QPluginLoader::instance() is
 * cached inside the library. If you call factory() or instance() multiple times, you will always get
 * the same object, even from different threads and different KPluginLoader instances.
 * You can delete this object easily, a new one will be created if factory() or instance() is called
 * afterwards. factory() uses instance() internally.
 *
 * KPluginLoader inherits QPluginLoader::unload(). It safe to call this methode if you loaded a plugin
 * and decide not to use it for some reason. But as soon as you start to use the factory from the plugin,
 * you should stay away from it. It's nearly impossible to keep track of all objects created directly or
 * indirectly from the plugin and all other pointers into plugin code. Using unload() in this case is asking
 * for trouble. If you really need to unload your plugins, you have to take care to convert the clipboard
 * content to text, because the plugin could have registered a custom mime source. You also have to delete
 * the factory of the plugin, otherwise you will create a leak.
 * The destructor of KPluginLoader doesn't call unload.
 *
 * \see KPluginFactory
 * 
 * \author Bernhard Loos <nhuh.put@web.de>
 */
class KDECORE_EXPORT KPluginLoader : public QPluginLoader
{
    Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName)
    Q_PROPERTY(QString pluginName READ fileName)
public:
    /**
     * Used this constructor to load a plugin with a given library name. Plugin libraries shouldn't have a 'lib' prefix.
     *
     * \param plugin The name of the plugin library.
     * \param componentdata The KStandardDirs object from componentdata is used to search the library.
     * \param parent A parent object.
     */
    explicit KPluginLoader(const QString &plugin, const KComponentData &componentdata = KGlobal::mainComponent(), QObject *parent = 0);
 
    /**
     * Used this constructor to load a plugin from a service. The service must contain a library.
     *
     * \param service The service for which the library should be loaded.
     * \param componentdata The KStandardDirs object from componentdata is used to search the library.
     * \param parent A parent object.
     */
    explicit KPluginLoader(const KService &service, const KComponentData &componentdata = KGlobal::mainComponent(), QObject *parent = 0);

    /**
     * Destroys the plugin loader.
     */
    ~KPluginLoader();

    /**
     * Used to obtain the factory object of the plugin. You have to give a class which inherits KPluginFactory
     * to K_EXPORT_PLUGIN to use this methode.
     * 
     * \returns The factory of the plugin or 0 on error.
     */
    KPluginFactory *factory();

    /**
     * The name of this plugin as given to the constructor.
     * \returns the plugin name
     */
    QString pluginName() const;

    /**
     * Queries the plugin version.
     * \returns The version given to K_EXPORT_PLUGIN_VERSION or (quint32) -1 if not set.
     */
    quint32 pluginVersion() const;
 
    /**
     * Queries the last error.
     * \returns The description of the last error.
     */
    QString errorString() const;

    bool isLoaded() const;

protected:
    /** 
     * Performs the loading of the plugin.
     */
    bool load();
private:
    Q_DECLARE_PRIVATE(KPluginLoader)
    Q_DISABLE_COPY(KPluginLoader)

    using QPluginLoader::setFileName;

    KPluginLoaderPrivate *const d_ptr;
};


#endif
