#include <kparts/plugin.h>
#include <kparts/part.h>

#include <qobjectlist.h>

#include <klibloader.h>
#include <kinstance.h>
#include <kstddirs.h>
#include <kdebug.h>

using namespace KParts;

Plugin::Plugin( QObject* parent, const char* name )
    : QObject( parent, name ), m_collection( this )
{
  qDebug( className() );
}

Plugin::~Plugin()
{
}

KAction* Plugin::action( const char* name )
{
    return m_collection.action( name );
}

KActionCollection* Plugin::actionCollection()
{
    return &m_collection;
}

KAction *Plugin::action( const QDomElement &element )
{
  return action( element.attribute( "name" ).latin1() );
}

void Plugin::setDocument( QDomDocument doc )
{
  m_doc = doc;
}

QDomDocument Plugin::document() const
{
  return m_doc;
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
       plugin->setDocument( *pIt );
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
