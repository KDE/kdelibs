#include "kplugin.h"
#include "kpart.h"

#include <qobjectlist.h>

#include <klibloader.h>
#include <kdebug.h>

using namespace KParts;

Plugin::Plugin( QObject* parent, const char* name )
    : QObject( parent, name ), m_collection( this )
{
}

Plugin::~Plugin()
{
}

QAction* Plugin::action( const char* name )
{
    return m_collection.action( name );
}

QActionCollection* Plugin::actionCollection()
{
    return &m_collection;
}

QAction *Plugin::action( const QDomElement &element )
{
  QAction *a = action( element.attribute( "name" ).latin1() ); 
  
  if ( !a && parent()->inherits( "KParts::Part" ) )
    a = ((Part *)parent() )->action( element );
  
  return a;
} 

void Plugin::setDocument( QDomDocument doc )
{
  m_doc = doc; 
}

QDomDocument Plugin::document() const
{
  return m_doc; 
} 


void Plugin::loadPlugins( QObject *parent, const QValueList<QDomDocument> pluginDocuments )
{
   QValueList<QDomDocument>::ConstIterator pIt = pluginDocuments.begin();
   QValueList<QDomDocument>::ConstIterator pEnd = pluginDocuments.end();
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
        kDebugError( 1000, "No library loader installed" );
        return 0;
    }

    KLibFactory* f = loader->factory( libname );
    if ( !f )
    {
        kDebugError( 1000, "Could not initialize library" );
        return 0;
    }
    QObject* obj = f->create( parent, libname, "Plugin" );
    if ( !obj->inherits("KParts::Plugin" ) )
    {
        kDebugError( 1000, "The library does not feature an object of class Plugin" );
        delete obj;
        return 0;
    }

    return (Plugin*)obj;
}

QValueList<XMLGUIServant *> Plugin::pluginServants( QObject *parent )
{
  QValueList<XMLGUIServant *> servants;
 
  if (!parent )
    return servants;
 
  QObjectList *plugins = parent->queryList( "KParts::Plugin", 0, false, false );
  
  QObjectListIt it( *plugins );
  while( it.current() )
    servants.append( (XMLGUIServant *)it.current() );
  
  return servants;
} 

#include "kplugin.moc"
