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

#include <qfile.h>
#include <qobjectlist.h>
#include <qfileinfo.h>

#include <klibloader.h>
#include <kinstance.h>
#include <kstddirs.h>
#include <kdebug.h>

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
          info.m_relXMLFileName = QString::fromLocal8Bit( instance->instanceName() ) + "/kpartplugins/" + mapIt.key();

          info.m_document.setContent( doc );
          if ( !info.m_document.documentElement().isNull() )
            plugins.append( info );
      }
  }

  return plugins;
}

void Plugin::loadPlugins( QObject *parent, const KInstance *instance )
{
  loadPlugins( parent, pluginInfos( instance ) );
}

void Plugin::loadPlugins( QObject *parent, const QValueList<PluginInfo> &pluginInfos )
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
       plugin->setXMLFile( (*pIt).m_absXMLFileName, false, false );
       plugin->setLocalXMLFile( locateLocal( "data", (*pIt).m_relXMLFileName ) );
       plugin->setDOMDocument( (*pIt).m_document );
     }
   }
}

// static
Plugin* Plugin::loadPlugin( QObject * parent, const char* libname )
{
    KLibLoader* loader = KLibLoader::self();
    if ( !loader )
    {
        kdError(1000) << "No library loader installed" << endl;
        return 0;
    }

    KLibFactory* f = loader->factory( libname );
    if ( !f )
    {
        kdError(1000) << "Could not initialize library" << endl;
        return 0;
    }
    QObject* obj = f->create( parent, libname, "KParts::Plugin" );
    if ( !obj->inherits("KParts::Plugin" ) )
    {
        kdError(1000) << "The library does not feature an object of class Plugin" << endl;
        delete obj;
        return 0;
    }

    return (Plugin*)obj;
}

QList<KParts::Plugin> Plugin::pluginObjects( QObject *parent )
{
  QList<KParts::Plugin> objects;

  if (!parent )
    return objects;

  QObjectList *plugins = parent->queryList( "KParts::Plugin", 0, false, false );

  QObjectListIt it( *plugins );
  while( it.current() )
  {
    objects.append( static_cast<Plugin *>( it.current() ) );
    ++it;
  }

  return objects;
}

#include "plugin.moc"
