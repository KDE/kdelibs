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

#include <qfile.h>
#include <qobjectlist.h>
#include <qfileinfo.h>

#include <klibloader.h>
#include <kinstance.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kxmlguifactory.h>
#include <klocale.h>

using namespace KParts;

namespace KParts
{
class Plugin::PluginPrivate
{
public:
  PluginPrivate()
  {
  }
  ~PluginPrivate()
  {
  }
};
};

Plugin::Plugin( QObject* parent, const char* name )
    : QObject( parent, name )
{
  kdDebug() << className() << endl;
//  d = new PluginPrivate();
}

Plugin::~Plugin()
{
    if ( factory() )
        factory()->removeClient( this );
//  delete d;
}

//static
QValueList<Plugin::PluginInfo> Plugin::pluginInfos( const KInstance * instance )
{
  if ( !instance )
    kdError(1000) << "No instance ???" << endl;

  QValueList<PluginInfo> plugins;

  QStringList pluginDocs = instance->dirs()->findAllResources(
    "data", instance->instanceName()+"/kpartplugins/*", true, false );

  QMap<QString,QStringList> sortedPlugins;

  QStringList::ConstIterator pIt = pluginDocs.begin();
  QStringList::ConstIterator pEnd = pluginDocs.end();
  for (; pIt != pEnd; ++pIt )
  {
      QFileInfo fInfo( *pIt );
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
      if ( !info.m_absXMLFileName.isEmpty() )
      {
          kdDebug( 1000 ) << "found Plugin : " << info.m_absXMLFileName << " !" << endl;
          info.m_relXMLFileName = "kpartplugins/";
          info.m_relXMLFileName += mapIt.key();

          info.m_document.setContent( doc );
          if ( !info.m_document.documentElement().isNull() )
            plugins.append( info );
      }
  }

  return plugins;
}

void Plugin::loadPlugins( QObject *parent, const KInstance *instance )
{
  loadPlugins( parent, pluginInfos( instance ), instance );
}

void Plugin::loadPlugins( QObject *parent, const QValueList<PluginInfo> &pluginInfos, const KInstance *instance )
{
   QValueList<PluginInfo>::ConstIterator pIt = pluginInfos.begin();
   QValueList<PluginInfo>::ConstIterator pEnd = pluginInfos.end();
   for (; pIt != pEnd; ++pIt )
   {
     QString library = (*pIt).m_document.documentElement().attribute( "library" );

     if ( library.isEmpty() )
       continue;

     Plugin *plugin = loadPlugin( parent, QFile::encodeName(library) );

     if ( plugin )
     {
       if ( instance )
         plugin->setInstance( (KInstance*)instance ); 
       plugin->setXMLFile( (*pIt).m_relXMLFileName, false, false );
       plugin->setDOMDocument( (*pIt).m_document );
     }
   }

}

void Plugin::loadPlugins( QObject *parent, const QValueList<PluginInfo> &pluginInfos )
{
   loadPlugins(parent, pluginInfos, 0); 
}

// static
Plugin* Plugin::loadPlugin( QObject * parent, const char* libname )
{
    return ComponentFactory::createInstanceFromLibrary<Plugin>( libname, parent, libname );
}

QPtrList<KParts::Plugin> Plugin::pluginObjects( QObject *parent )
{
  QPtrList<KParts::Plugin> objects;

  if (!parent )
    return objects;

  QObjectList *plugins = parent->queryList( "KParts::Plugin", 0, false, false );

  QObjectListIt it( *plugins );
  while( it.current() )
  {
    objects.append( static_cast<Plugin *>( it.current() ) );
    ++it;
  }

  delete plugins;

  return objects;
}

void Plugin::setInstance( KInstance *instance )
{
    KGlobal::locale()->insertCatalogue( instance->instanceName() );
}


#include "plugin.moc"
