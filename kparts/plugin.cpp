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

#include <kparts/plugin.h>
#include <kparts/part.h>

#include <qobjectlist.h>

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
const QValueList<QDomDocument> Plugin::pluginDocuments( const KInstance * instance )
{
  if ( !instance )
    kdError(1000) << "No instance ???" << endl;

  QValueList<QDomDocument> docs;

  QStringList pluginDocs = instance->dirs()->findAllResources(
    "data", instance->instanceName()+"/kpartplugins/*", true, false );

  QStringList::ConstIterator pIt = pluginDocs.begin();
  QStringList::ConstIterator pEnd = pluginDocs.end();
    for (; pIt != pEnd; ++pIt )
    {
      kdDebug(1000) << "Plugin : " << (*pIt) << endl;
      QString xml = KXMLGUIFactory::readConfigFile( *pIt );
      if ( !xml.isEmpty() )
      {
        QDomDocument doc;
        doc.setContent( xml );
        if ( !doc.documentElement().isNull() )
          docs.append( doc );
      }
    }

  return docs;
}

void Plugin::loadPlugins( QObject *parent, const KInstance *instance )
{
  loadPlugins( parent, pluginDocuments( instance ) );
}

void Plugin::loadPlugins( QObject *parent, const QValueList<QDomDocument> &docs )
{
   QValueList<QDomDocument>::ConstIterator pIt = docs.begin();
   QValueList<QDomDocument>::ConstIterator pEnd = docs.end();
   for (; pIt != pEnd; ++pIt )
   {
     QString library = (*pIt).documentElement().attribute( "library" );

     if ( library.isEmpty() )
       continue;

     Plugin *plugin = Plugin::loadPlugin( parent, library.latin1() );

     if ( plugin )
       plugin->setDOMDocument( *pIt );
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

QValueList<KXMLGUIClient *> Plugin::pluginClients( QObject *parent )
{
  QValueList<KXMLGUIClient *> clients;

  if (!parent )
    return clients;

  QObjectList *plugins = parent->queryList( "KParts::Plugin", 0, false, false );

  QObjectListIt it( *plugins );
  while( it.current() )
  {
    clients.append( (KXMLGUIClient *)((Plugin *)it.current()) );
    ++it;
  }

  return clients;
}

#include "plugin.moc"
