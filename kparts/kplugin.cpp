#include "kplugin.h"
#include "kpart.h"

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
