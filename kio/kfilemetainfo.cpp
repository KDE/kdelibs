#include <kfileitem.h>
#include <kstaticdeleter.h>
#include <kparts/componentfactory.h>
#include <kuserprofile.h>

#include "kfilemetainfo.h"

KFileMetaInfoItem::KFileMetaInfoItem( const QString& key, 
                                      const QString& translatedKey, 
                                      const QVariant& value, 
                                      bool editable, 
                                      const QString& prefix, 
                                      const QString& postfix )
    : m_key( key ),
      m_translatedKey( translatedKey ),
      m_prefix( prefix ),
      m_postfix( postfix ),
      m_value( value ),
      m_editable( editable ),
      m_dirty( false )
{
}

KFileMetaInfoItem::~KFileMetaInfoItem()
{
}
    
void KFileMetaInfoItem::setValue( const QVariant& value )
{
    if ( !isEditable() )
        return;

    m_dirty = (value != m_value);
    m_value = value;
}


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

// this is one object per file

KFileMetaInfo::KFileMetaInfo( const KURL& url )
    : m_url( url )
{
}

KFileMetaInfo::~KFileMetaInfo()
{
}
    
KFileMetaInfoItem * KFileMetaInfo::item( const QString& key ) const
{
    return m_items[key];
}

QStringList KFileMetaInfo::preferredKeys() const
{
    return QStringList();
}
    
bool KFileMetaInfo::supportsVariableKeys() const
{
    return false;
}

void KFileMetaInfo::applyChanges()
{
}
    
QValidator * KFileMetaInfo::createValidator( const QString& key, 
                                             QObject *parent, 
                                             const char *name ) const
{
    return 0L;
}


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

KFilePlugin::KFilePlugin( QObject *parent, const char *name, const QStringList& preferredItems )
{
    qDebug("### KFilePlugin::KFilePlugin!");
}

KFilePlugin::~KFilePlugin()
{
    qDebug("## ~KFilePlugin()");
}


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


KFileMetaInfoProvider * KFileMetaInfoProvider::s_self = 0L;
KStaticDeleter<KFileMetaInfoProvider> sd;

KFileMetaInfoProvider * KFileMetaInfoProvider::self()
{
    if ( !s_self )
        s_self = sd.setObject( new KFileMetaInfoProvider() );

    return s_self;
}

KFileMetaInfoProvider::KFileMetaInfoProvider()
{
    m_plugins.setAutoDelete( true );
}

KFileMetaInfoProvider::~KFileMetaInfoProvider()
{
}
    

KFileMetaInfo * KFileMetaInfoProvider::metaInfo( const KFileItem *item )
{
    return metaInfo( item->url(), item->mimetype() );
}

KFileMetaInfo * KFileMetaInfoProvider::metaInfo( const KURL& url )
{
    return metaInfo( url, 
                     KMimeType::findByURL(url, 0, url.isLocalFile())->name() );
}

KFileMetaInfo * KFileMetaInfoProvider::metaInfo( const KURL& url, 
                                                 const QString& mimeType )
{
    KFilePlugin *plugin = m_plugins.find( mimeType );

    if ( !plugin ) {
        KService::Ptr service = 
            KServiceTypeProfile::preferredService( mimeType, "KFilePlugin");
        if ( !service->isValid() )
            return 0L;

        // ### this needs fixing
        QVariant preferredItems = service->property("PreferredItems");
        plugin = KParts::ComponentFactory::createInstanceFromService<KFilePlugin>
                 ( service, 0L, 0L, preferredItems.toStringList() );
        
        if ( !plugin )
            return 0L;
        
        m_plugins.insert( mimeType, plugin );
    }

    return plugin->createInfo( url ); // ### who cleans up?
}


#include "kfilemetainfo.moc"
