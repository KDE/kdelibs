#include "kplugin.h"
#include "kpart.h"
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
    if ( !obj->inherits("KPlugin" ) )
    {
        kDebugError( 1000, "The library does not feature an object of class Plugin" );
        delete obj;
        return 0;
    }

    return (Plugin*)obj;
}

//////////////

PluginGUIServant::PluginGUIServant( Part *part, const QDomDocument &document )
  : QObject( part )
{
  m_part = part;
  m_doc = document;
}

QAction *PluginGUIServant::action( const QDomElement &element )
{
  return m_part->action( element );
}

QDomDocument PluginGUIServant::document() const
{
  return m_doc;
}

#include "kplugin.moc"
