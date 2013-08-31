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

#include <kparts/plugin.h>
#include <kparts/part.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>

#include <assert.h>

#include <QtCore/QFile>
#include <QtCore/QObject>
#include <QtCore/QFileInfo>
#include <QDir>

#include <kaboutdata.h>
#include <QDebug>
#include <kxmlguifactory.h>
#include <klocalizedstring.h>
#include <kdesktopfile.h>
#include <kconfiggroup.h>
#include <qstandardpaths.h>

using namespace KParts;

class Plugin::PluginPrivate
{
public:
    QString m_parentInstance;
    QString m_library; // filename of the library
};

Plugin::Plugin( QObject* parent )
    : QObject( parent ),d(new PluginPrivate())
{
  //qDebug() << className();
}

Plugin::~Plugin()
{
    delete d;
}

QString Plugin::xmlFile() const
{
    QString path = KXMLGUIClient::xmlFile();

    if (d->m_parentInstance.isEmpty() || (!path.isEmpty() && QDir::isAbsolutePath(path)))
        return path;

    QString absPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, d->m_parentInstance + QLatin1Char('/') + path );
    assert( !absPath.isEmpty() );
    return absPath;
}

QString Plugin::localXMLFile() const
{
    QString path = KXMLGUIClient::xmlFile();

    if (d->m_parentInstance.isEmpty() || (!path.isEmpty() && QDir::isAbsolutePath(path)))
        return path;

    QString absPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + d->m_parentInstance + QLatin1Char('/') + path;
    return absPath;
}

//static
QList<Plugin::PluginInfo> Plugin::pluginInfos(const QString &componentName)
{
  QList<PluginInfo> plugins;

    QMap<QString,QStringList> sortedPlugins;

    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, componentName + QStringLiteral("/kpartplugins"), QStandardPaths::LocateDirectory);
    Q_FOREACH(const QString& dir, dirs) {
        Q_FOREACH(const QString& file, QDir(dir).entryList(QStringList() << QLatin1String("*.rc"))) {
            const QFileInfo fInfo(dir + QLatin1Char('/') + file);
            QMap<QString,QStringList>::Iterator mapIt = sortedPlugins.find(fInfo.fileName());
            if (mapIt == sortedPlugins.end())
                mapIt = sortedPlugins.insert(fInfo.fileName(), QStringList());
            mapIt.value().append(fInfo.absoluteFilePath());
        }
    }

  QMap<QString,QStringList>::ConstIterator mapIt = sortedPlugins.constBegin();
  QMap<QString,QStringList>::ConstIterator mapEnd = sortedPlugins.constEnd();
  for (; mapIt != mapEnd; ++mapIt )
  {
      PluginInfo info;
      QString doc;
      info.m_absXMLFileName = KXMLGUIClient::findMostRecentXMLFile( mapIt.value(), doc );
      if ( info.m_absXMLFileName.isEmpty() )
          continue;

      // qDebug() << "found KParts Plugin : " << info.m_absXMLFileName;
      info.m_relXMLFileName = QStringLiteral("kpartplugins/") + mapIt.key();

      info.m_document.setContent( doc );
      if ( info.m_document.documentElement().isNull() )
          continue;

      plugins.append( info );
  }

  return plugins;
}

void Plugin::loadPlugins(QObject *parent, const QString &componentName)
{
  loadPlugins( parent, pluginInfos( componentName ), componentName );
}

void Plugin::loadPlugins(QObject *parent, const QList<PluginInfo> &pluginInfos, const QString &componentName)
{
   QList<PluginInfo>::ConstIterator pIt = pluginInfos.begin();
   QList<PluginInfo>::ConstIterator pEnd = pluginInfos.end();
   for (; pIt != pEnd; ++pIt )
   {
     QString library = (*pIt).m_document.documentElement().attribute(QStringLiteral("library"));

     if ( library.isEmpty() || hasPlugin( parent, library ) )
       continue;

     Plugin *plugin = loadPlugin(parent, library, (*pIt).m_document.documentElement().attribute(QStringLiteral("X-KDE-PluginKeyword")));

     if ( plugin )
     {
       plugin->d->m_parentInstance = componentName;
       plugin->setXMLFile( (*pIt).m_relXMLFileName, false, false );
       plugin->setDOMDocument( (*pIt).m_document );

     }
   }

}

void Plugin::loadPlugins( QObject *parent, const QList<PluginInfo> &pluginInfos )
{
   loadPlugins(parent, pluginInfos, QString());
}

// static
Plugin* Plugin::loadPlugin( QObject * parent, const QString &libname, const QString &keyword )
{
    KPluginLoader loader( libname );
    KPluginFactory* factory = loader.factory();

    if (!factory) {
        return 0;
    }

    Plugin* plugin = factory->create<Plugin>( keyword, parent );
    if ( !plugin )
        return 0;
    plugin->d->m_library = libname;
    return plugin;
}

QList<KParts::Plugin *> Plugin::pluginObjects( QObject *parent )
{
  QList<KParts::Plugin *> objects;

  if (!parent )
    return objects;

  objects = parent->findChildren<Plugin *>(QString(), Qt::FindDirectChildrenOnly);
  return objects;
}

bool Plugin::hasPlugin( QObject* parent, const QString& library )
{
  const QObjectList plugins = parent->children();

  QObjectList::ConstIterator it = plugins.begin();
  for ( ; it != plugins.end() ; ++it )
  {
      Plugin * plugin = qobject_cast<Plugin *>( *it );
      if ( plugin && plugin->d->m_library == library )
      {
          return true;
      }
  }
  return false;
}

void Plugin::setComponentData(const KAboutData &pluginData)
{
    KAboutData::registerPluginData(pluginData);
    KXMLGUIClient::setComponentName(pluginData.componentName(), pluginData.displayName());
}

void Plugin::loadPlugins(QObject *parent, KXMLGUIClient* parentGUIClient,
        const QString &componentName, bool enableNewPluginsByDefault,
        int interfaceVersionRequired)
{
    KConfigGroup cfgGroup(KSharedConfig::openConfig(componentName + QStringLiteral("rc")), "KParts Plugins");
    const QList<PluginInfo> plugins = pluginInfos( componentName );
    QList<PluginInfo>::ConstIterator pIt = plugins.begin();
    const QList<PluginInfo>::ConstIterator pEnd = plugins.end();
    for (; pIt != pEnd; ++pIt )
    {
        QDomElement docElem = (*pIt).m_document.documentElement();
        QString library = docElem.attribute(QStringLiteral("library"));
        QString keyword;

        if ( library.isEmpty() )
            continue;

        // Check configuration
        const QString name = docElem.attribute(QStringLiteral("name"));

        bool pluginEnabled = enableNewPluginsByDefault;
        if ( cfgGroup.hasKey(name + QStringLiteral("Enabled"))) {
            pluginEnabled = cfgGroup.readEntry(name + QStringLiteral("Enabled"), false);
        } else { // no user-setting, load plugin default setting
            QString relPath = componentName + QLatin1Char('/') + (*pIt).m_relXMLFileName;
            relPath.truncate(relPath.lastIndexOf(QLatin1Char('.'))); // remove extension
            relPath += QStringLiteral(".desktop");
            //qDebug() << "looking for " << relPath;
            const QString desktopfile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, relPath);
            if (!desktopfile.isEmpty())
            {
                //qDebug() << "loadPlugins found desktop file for " << name << ": " << desktopfile;
                KDesktopFile _desktop( desktopfile );
                const KConfigGroup desktop = _desktop.desktopGroup();
                keyword = desktop.readEntry("X-KDE-PluginKeyword", "");
                pluginEnabled = desktop.readEntry( "X-KDE-PluginInfo-EnabledByDefault",
                                                   enableNewPluginsByDefault );
                if ( interfaceVersionRequired != 0 )
                {
                    const int version = desktop.readEntry( "X-KDE-InterfaceVersion", 1 );
                    if ( version != interfaceVersionRequired )
                    {
                        // qDebug() << "Discarding plugin " << name << ", interface version " << version << ", expected " << interfaceVersionRequired;
                        pluginEnabled = false;
                    }
                }
            }
            else
            {
                //qDebug() << "loadPlugins no desktop file found in " << relPath;
            }
        }

        // search through already present plugins
        const QObjectList pluginList = parent->children();

        bool pluginFound = false;
        for ( QObjectList::ConstIterator it = pluginList.begin(); it != pluginList.end() ; ++it )
        {
            Plugin * plugin = qobject_cast<Plugin *>( *it );
            if( plugin && plugin->d->m_library == library )
            {
                // delete and unload disabled plugins
                if( !pluginEnabled )
                {
                    // qDebug() << "remove plugin " << name;
                    KXMLGUIFactory * factory = plugin->factory();
                    if( factory )
                        factory->removeClient( plugin );
                    delete plugin;
                }

                pluginFound = true;
                break;
            }
        }

        // if the plugin is already loaded or if it's disabled in the
        // configuration do nothing
        if( pluginFound || !pluginEnabled )
            continue;

        // qDebug() << "load plugin " << name << " " << library << " " << keyword;
        Plugin *plugin = loadPlugin( parent, library, keyword );

        if ( plugin )
        {
            plugin->d->m_parentInstance = componentName;
            plugin->setXMLFile( (*pIt).m_relXMLFileName, false, false );
            plugin->setDOMDocument( (*pIt).m_document );
            parentGUIClient->insertChildClient( plugin );
        }
    }
}

// vim:sw=4:et:sts=4

