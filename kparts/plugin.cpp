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
#include <kparts/componentfactory.h>

#include <assert.h>

#include <QtCore/QFile>
#include <QtCore/QObject>
#include <QtCore/QFileInfo>

#include <klibloader.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kxmlguifactory.h>
#include <klocale.h>
#include <kdesktopfile.h>
#include <kconfiggroup.h>

using namespace KParts;

class Plugin::PluginPrivate
{
public:
    KComponentData m_parentInstance;
    QString m_library; // filename of the library
};

Plugin::Plugin( QObject* parent )
    : QObject( parent ),d(new PluginPrivate())
{
  //kDebug() << className();
}

Plugin::~Plugin()
{
    delete d;
}

QString Plugin::xmlFile() const
{
    QString path = KXMLGUIClient::xmlFile();

    if ( !d->m_parentInstance.isValid() || ( path.length() > 0 && path[ 0 ] == '/' ) )
        return path;

    QString absPath = KStandardDirs::locate( "data", d->m_parentInstance.componentName() + '/' + path );
    assert( !absPath.isEmpty() );
    return absPath;
}

QString Plugin::localXMLFile() const
{
    QString path = KXMLGUIClient::xmlFile();

    if ( !d->m_parentInstance.isValid() || ( path.length() > 0 && path[ 0 ] == '/' ) )
        return path;

    QString absPath = KStandardDirs::locateLocal( "data", d->m_parentInstance.componentName() + '/' + path );
    assert( !absPath.isEmpty() );
    return absPath;
}

//static
QList<Plugin::PluginInfo> Plugin::pluginInfos(const KComponentData &componentData)
{
  if (!componentData.isValid())
    kError(1000) << "No componentData ???" << endl;

  QList<PluginInfo> plugins;

  // KDE4: change * into *.rc and remove test for .desktop from the for loop below.
  const QStringList pluginDocs = componentData.dirs()->findAllResources(
    "data", componentData.componentName()+"/kpartplugins/*", KStandardDirs::Recursive );

  QMap<QString,QStringList> sortedPlugins;

  QStringList::ConstIterator pIt = pluginDocs.begin();
  QStringList::ConstIterator pEnd = pluginDocs.end();
  for (; pIt != pEnd; ++pIt )
  {
      QFileInfo fInfo( *pIt );
      if ( fInfo.completeSuffix() == QLatin1String( "desktop" ) )
          continue;

      QMap<QString,QStringList>::Iterator mapIt = sortedPlugins.find( fInfo.fileName() );
      if ( mapIt == sortedPlugins.end() )
          mapIt = sortedPlugins.insert( fInfo.fileName(), QStringList() );

      mapIt.value().append( *pIt );
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

      kDebug( 1000 ) << "found KParts Plugin : " << info.m_absXMLFileName;
      info.m_relXMLFileName = "kpartplugins/";
      info.m_relXMLFileName += mapIt.key();

      info.m_document.setContent( doc );
      if ( info.m_document.documentElement().isNull() )
          continue;

      plugins.append( info );
  }

  return plugins;
}

void Plugin::loadPlugins(QObject *parent, const KComponentData &componentData)
{
  loadPlugins( parent, pluginInfos( componentData ), componentData );
}

void Plugin::loadPlugins(QObject *parent, const QList<PluginInfo> &pluginInfos, const KComponentData &componentData)
{
   QList<PluginInfo>::ConstIterator pIt = pluginInfos.begin();
   QList<PluginInfo>::ConstIterator pEnd = pluginInfos.end();
   for (; pIt != pEnd; ++pIt )
   {
     QString library = (*pIt).m_document.documentElement().attribute( "library" );
     
     if ( library.isEmpty() || hasPlugin( parent, library ) )
       continue;

     Plugin *plugin = loadPlugin( parent, library, (*pIt).m_document.documentElement().attribute( "X-KDE-PluginKeyword" ) );

     if ( plugin )
     {
       plugin->d->m_parentInstance = componentData;
       plugin->setXMLFile( (*pIt).m_relXMLFileName, false, false );
       plugin->setDOMDocument( (*pIt).m_document );

     }
   }

}

void Plugin::loadPlugins( QObject *parent, const QList<PluginInfo> &pluginInfos )
{
   loadPlugins(parent, pluginInfos, KComponentData());
}

// static, deprecated
Plugin* Plugin::loadPlugin( QObject * parent, const char* libname )
{
    Plugin* plugin = KLibLoader::createInstance<Plugin>( libname, parent );
    if ( !plugin )
        return 0;
    plugin->d->m_library = libname;
    return plugin;
}

// static, deprecated
Plugin* Plugin::loadPlugin( QObject * parent, const QByteArray &libname )
{
    return loadPlugin( parent, libname.data() );
}

Plugin* Plugin::loadPlugin( QObject * parent, const QString &libname )
{
    return loadPlugin( parent, libname, "" );
}

// static
Plugin* Plugin::loadPlugin( QObject * parent, const QString &libname, const QString &keyword )
{
    Plugin* plugin = KLibLoader::createInstance<Plugin>( keyword, libname, parent );
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

  const QObjectList plugins = parent->children();

  QObjectList::ConstIterator it = plugins.begin();
  for ( ; it != plugins.end() ; ++it )
  {
    Plugin * plugin = qobject_cast<Plugin *>( *it );
    if ( plugin )
        objects.append( plugin );
  }

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

void Plugin::setComponentData(const KComponentData &componentData)
{
    KGlobal::locale()->insertCatalog(componentData.catalogName());
    KXMLGUIClient::setComponentData(componentData);
}

void Plugin::loadPlugins(QObject *parent, KXMLGUIClient* parentGUIClient,
        const KComponentData &componentData, bool enableNewPluginsByDefault,
        int interfaceVersionRequired)
{
    KConfigGroup cfgGroup( componentData.config(), "KParts Plugins" );
    const QList<PluginInfo> plugins = pluginInfos( componentData );
    QList<PluginInfo>::ConstIterator pIt = plugins.begin();
    const QList<PluginInfo>::ConstIterator pEnd = plugins.end();
    for (; pIt != pEnd; ++pIt )
    {
        QDomElement docElem = (*pIt).m_document.documentElement();
        QString library = docElem.attribute( "library" );
        QString keyword;

        if ( library.isEmpty() )
            continue;

        // Check configuration
        const QString name = docElem.attribute( "name" );

        bool pluginEnabled = enableNewPluginsByDefault;
        if ( cfgGroup.hasKey( name + "Enabled" ) )
        {
            pluginEnabled = cfgGroup.readEntry( name + "Enabled" , false );
        }
        else
        { // no user-setting, load plugin default setting
            QString relPath = QString( componentData.componentName() ) + '/' + (*pIt).m_relXMLFileName;
            relPath.truncate( relPath.lastIndexOf( '.' ) ); // remove extension
            relPath += ".desktop";
            //kDebug(1000) << "looking for " << relPath;
            const QString desktopfile = componentData.dirs()->findResource( "data", relPath );
            if( !desktopfile.isEmpty() )
            {
                //kDebug(1000) << "loadPlugins found desktop file for " << name << ": " << desktopfile;
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
                        kDebug(1000) << "Discarding plugin " << name << ", interface version " << version << ", expected " << interfaceVersionRequired;
                        pluginEnabled = false;
                    }
                }
            }
            else
            {
                //kDebug(1000) << "loadPlugins no desktop file found in " << relPath;
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
                    kDebug( 1000 ) << "remove plugin " << name;
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

        kDebug( 1000 ) << "load plugin " << name << " " << library << " " << keyword;
        Plugin *plugin = loadPlugin( parent, library, keyword );

        if ( plugin )
        {
            plugin->d->m_parentInstance = componentData;
            plugin->setXMLFile( (*pIt).m_relXMLFileName, false, false );
            plugin->setDOMDocument( (*pIt).m_document );
            parentGUIClient->insertChildClient( plugin );
        }
    }
}

// vim:sw=4:et:sts=4

#include "plugin.moc"
