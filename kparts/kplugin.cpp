#include "kplugin.h"

KPlugin::KPlugin( QObject* parent, const char* name )
    : QObject( parent, name ), m_collection( this )
{
}

KPlugin::~KPlugin()
{
}

QAction* KPlugin::action( const char* name )
{
    return m_collection.action( name );
}

QActionCollection* KPlugin::actionCollection()
{
    return &m_collection;
}

#include "kplugin.moc"
