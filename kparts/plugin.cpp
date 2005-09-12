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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>
#include <kparts/plugin.h>
#include <kparts/part.h>
#include <kparts/componentfactory.h>

#include <assert.h>

#include <qfile.h>
#include <qobject.h>
#include <qfileinfo.h>

#include <klibloader.h>
#include <kinstance.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kxmlguifactory.h>
#include <klocale.h>
#include <kconfig.h>
#include <ksimpleconfig.h>

using namespace KParts;

class Plugin::PluginPrivate
{
public:
    PluginPrivate() : m_parentInstance( 0 ) {}

    const KInstance *m_parentInstance;
    QString m_library; // filename of the library
};

Plugin::Plugin( QObject* parent )
    : QObject( parent )
{
  //kdDebug() << className() << endl;
  d = new PluginPrivate();
}

Plugin::~Plugin()
{
    delete d;
}

QString Plugin::xmlFile() const
{
    QString path = KXMLGUIClient::xmlFile();

    if ( !d->m_parentInstance || ( path.length() > 0 && path[ 0 ] == '/' ) )
        return path;

    QString absPath = locate( "data", QString::fromLatin1( d->m_parentInstance->instanceName() ) + '/' + path );
    assert( !absPath.isEmpty() );
    return absPath;
}

QString Plugin::localXMLFile() const
{
    QString path = KXMLGUIClient::xmlFile();

    if ( !d->m_parentInstance || ( path.length() > 0 && path[ 0 ] == '/' ) )
        return path;

    QString absPath = locateLocal( "data", QString::fromLatin1( d->m_parentInstance->instanceName() ) + '/' + path );
    assert( !absPath.isEmpty() );
    return absPath;
}

//static
Q3ValueList<Plugin::PluginInfo> Plugin::pluginInfos( const KInstance * instance )
{
  if ( !instance )
    kdError(1000) << "No instance ???" << endl;

  Q3ValueList<PluginInfo> plugins;

  // KDE4: change * into *.rc and remove test for .desktop from the for loop below.
  const QStringList pluginDocs = instance->dirs()->findAllResources(
    "data", instance->instanceName()+"/kpartplugins/*", true, false );

  QMap<QString,QStringList> sortedPlugins;

  QStringList::ConstIterator pIt = pluginDocs.begin();
  QStringList::ConstIterator pEnd = pluginDocs.end();
  for (; pIt != pEnd; ++pIt )
  {
      QFileInfo fInfo( *pIt );
      if ( fInfo.extension() == QString::fromLatin1( "desktop" ) )
          continue;

      QMap<QString,QStringList>::Iterator mapIt = sortedPlugins.find( fInfo.fileName() );
      if ( mapIt == sortedPlugins.end() )
          mapIt = sortedPlugins.insert( fInfo.fileName(), QStringList() );

      mapIt.data().append( *pIt );
  }

  QMap<QString,QStringList>::ConstIterator mapIt = sortedPlugins.begin();
  QMap<QString,QStringList>::ConstIterator mapEnd = sortedPlugins.end();
  for (; mapIt != mapEnd; ++mapIt )
  {
      PluginInfo info;
      QString doc;
      info.m_absXMLFileName = KXMLGUIClient::findMostRecentXMLFile( mapIt.data(), doc );
      if ( info.m_absXMLFileName.isEmpty() )
          continue;

      kdDebug( 1000 ) << "found KParts Plugin : " << info.m_absXMLFileName << endl;
      info.m_relXMLFileName = "kpartplugins/";
      info.m_relXMLFileName += mapIt.key();

      info.m_document.setContent( doc );
      if ( info.m_document.documentElement().isNull() )
          continue;

      plugins.append( info );
  }

  return plugins;
}

void Plugin::loadPlugins( QObject *parent, const KInstance *instance )
{
  loadPlugins( parent, pluginInfos( instance ), instance );
}

void Plugin::loadPlugins( QObject *parent, const Q3ValueList<PluginInfo> &pluginInfos, const KInstance *instance )
{
   Q3ValueList<PluginInfo>::ConstIterator pIt = pluginInfos.begin();
   Q3ValueList<PluginInfo>::ConstIterator pEnd = pluginInfos.end();
   for (; pIt != pEnd; ++pIt )
   {
     QString library = (*pIt).m_document.documentElement().attribute( "library" );

     if ( library.isEmpty() || hasPlugin( parent, library ) )
       continue;

     Plugin *plugin = loadPlugin( parent, QFile::encodeName(library) );

     if ( plugin )
     {
       plugin->d->m_parentInstance = instance;
       plugin->setXMLFile( (*pIt).m_relXMLFileName, false, false );
       plugin->setDOMDocument( (*pIt).m_document );

     }
   }

}

void Plugin::loadPlugins( QObject *parent, const Q3ValueList<PluginInfo> &pluginInfos )
{
   loadPlugins(parent, pluginInfos, 0);
}

// static
Plugin* Plugin::loadPlugin( QObject * parent, const char* libname )
{
    Plugin* plugin = ComponentFactory::createInstanceFromLibrary<Plugin>( libname, parent, libname );
    if ( !plugin )
        return 0L;
    plugin->d->m_library = libname;
    return plugin;
}

Q3PtrList<KParts::Plugin> Plugin::pluginObjects( QObject *parent )
{
  Q3PtrList<KParts::Plugin> objects;

  if (!parent )
    return objects;

  const QObjectList plugins = parent->queryList( "KParts::Plugin", 0, false, false );

  QObjectList::ConstIterator it = plugins.begin();
  for ( ; it != plugins.end() ; ++it )
  {
    objects.append( static_cast<Plugin *>( *it ) );
  }

  return objects;
}

bool Plugin::hasPlugin( QObject* parent, const QString& library )
{
  const QObjectList plugins = parent->queryList( "KParts::Plugin", 0, false, false );

  QObjectList::ConstIterator it = plugins.begin();
  for ( ; it != plugins.end() ; ++it )
  {
      if ( static_cast<Plugin *>( *it )->d->m_library == library )
      {
          return true;
      }
  }
  return false;
}

void Plugin::setInstance( KInstance *instance )
{
    KGlobal::locale()->insertCatalog( instance->instanceName() );
    KXMLGUIClient::setInstance( instance );
}

void Plugin::loadPlugins( QObject *parent, KXMLGUIClient* parentGUIClient, KInstance* instance, bool enableNewPluginsByDefault )
{
    KConfigGroup cfgGroup( instance->config(), "KParts Plugins" );
    Q3ValueList<PluginInfo> plugins = pluginInfos( instance );
    Q3ValueList<PluginInfo>::ConstIterator pIt = plugins.begin();
    Q3ValueList<PluginInfo>::ConstIterator pEnd = plugins.end();
    for (; pIt != pEnd; ++pIt )
    {
        QDomElement docElem = (*pIt).m_document.documentElement();
        QString library = docElem.attribute( "library" );

        if ( library.isEmpty() )
            continue;

        // Check configuration
        const QString name = docElem.attribute( "name" );

        bool pluginEnabled = enableNewPluginsByDefault;
        if ( cfgGroup.hasKey( name + "Enabled" ) )
        {
            pluginEnabled = cfgGroup.readBoolEntry( name + "Enabled" );
        }
        else
        { // no user-setting, load plugin default setting
            QString relPath = QString( instance->instanceName() ) + "/" + (*pIt).m_relXMLFileName;
            relPath.truncate( relPath.findRev( '.' ) ); // remove extension
            relPath += ".desktop";
            //kdDebug(1000) << "looking for " << relPath << endl;
            const QString desktopfile = instance->dirs()->findResource( "data", relPath );
            if( !desktopfile.isEmpty() )
            {
                //kdDebug(1000) << "loadPlugins found desktop file for " << name << ": " << desktopfile << endl;
                KSimpleConfig desktop( desktopfile, true );
                desktop.setDesktopGroup();
                pluginEnabled = desktop.readBoolEntry(
                    "X-KDE-PluginInfo-EnabledByDefault", enableNewPluginsByDefault );
            }
            else
            {
                //kdDebug(1000) << "loadPlugins no desktop file found in " << relPath << endl;
            }
        }

        // search through already present plugins
        QObjectList pluginList = parent->queryList( "KParts::Plugin", 0, false, false );

        bool pluginFound = false;
        for ( QObjectList::ConstIterator it = pluginList.begin(); it != pluginList.end() ; ++it )
        {
            Plugin * plugin = static_cast<Plugin *>( *it );
            if( plugin->d->m_library == library )
            {
                // delete and unload disabled plugins
                if( !pluginEnabled )
                {
                    kdDebug( 1000 ) << "remove plugin " << name << endl;
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

        kdDebug( 1000 ) << "load plugin " << name << endl;
        Plugin *plugin = loadPlugin( parent, QFile::encodeName(library) );

        if ( plugin )
        {
            plugin->d->m_parentInstance = instance;
            plugin->setXMLFile( (*pIt).m_relXMLFileName, false, false );
            plugin->setDOMDocument( (*pIt).m_document );
            parentGUIClient->insertChildClient( plugin );
        }
    }
}

// vim:sw=4:et:sts=4

#include "plugin.moc"
