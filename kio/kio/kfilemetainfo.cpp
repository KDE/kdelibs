#include <qshared.h>

#include <kfileitem.h>
#include <kstaticdeleter.h>
#include <kparts/componentfactory.h>
#include <kuserprofile.h>
#include <kdebug.h>

#include "kfilemetainfo.h"

// shared data of a KFileMetaInfoItem
class KFileMetaInfoItem::Data : public QShared
{
public:
    Data( const QString& _key, const QString& _translatedKey, const QVariant& _value, 
          const QString& _prefix, const QString& _suffix, const bool _editable)
        : QShared(),
          dirty( false ),
          key( _key ),
          translatedKey( _translatedKey ),
          prefix( _prefix ),
          suffix( _suffix ),
          value( _value ),
          editable( _editable )
    
    {}

    QString             key;
    QString             translatedKey;

    QString             prefix;
    QString             suffix;

    QVariant            value;

    bool                dirty    :1;
    bool                editable :1;
    
    static Data null;
};

//this is our null data
KFileMetaInfoItem::Data
KFileMetaInfoItem::Data::null(QString::null, QString::null, QVariant(),
                              QString::null, QString::null, false);


KFileMetaInfoItem::KFileMetaInfoItem( const QString& key,
                                      const QString& translatedKey,
                                      const QVariant& value,
                                      bool editable,
                                      const QString& prefix,
                                      const QString& suffix )
    : d( new Data(key, translatedKey, value, prefix, suffix, editable))
{
}

KFileMetaInfoItem::KFileMetaInfoItem( const KFileMetaInfoItem& item )
{
//    kdDebug(7033) << "KFileMetaInfoItem copy constructor\n";
    *this = item;
    // d gets copied by value, i.e. the pointer is shared!
    if (d != &Data::null) d->ref();
}

KFileMetaInfoItem::KFileMetaInfoItem()
{
//    kdDebug(7033) << "KFileMetaInfoItem default constructor\n";
    d = &Data::null;
}

KFileMetaInfoItem::~KFileMetaInfoItem()
{
//    kdDebug(7033) << "KFileMetaInfoItem destructor\n";
    if ( d != &Data::null && d->deref() )
    {
        kdDebug(7033) << "a metainfoitem " << d->key << " is finally deleted\n";
        delete d;
    }
}

const KFileMetaInfoItem& KFileMetaInfoItem::operator=
                                              (const KFileMetaInfoItem & item )
{
    d = item.d;
    d->ref();
//    kdDebug(7033) << "operator=\n";
    return *this;
}

void KFileMetaInfoItem::setValue( const QVariant& value )
{
    kdDebug(7033) << "KFileMetaInfoItem::setValue() of item " << d->key << endl;
    kdDebug(7033) << "isEditable: " << (d->editable ? "yes" : "no") << endl;
    kdDebug(7033) << "type: " << value.typeName() << " and " << d->value.typeName()
              << endl;
    
    if ( d == &Data::null ) return;
    
    if ( !d->editable ||
         (d->value.isValid() && value.type() != d->value.type()) )
        return;

    d->dirty = true;
    d->value = value;
}

void KFileMetaInfoItem::remove()
{
    d->value.clear();
    d->dirty = true;
}

bool KFileMetaInfoItem::isRemoved() const
{
    return (d->dirty && !d->value.isValid());
}

const QString& KFileMetaInfoItem::key() const
{
    return d->key;
}
  
const QString& KFileMetaInfoItem::translatedKey() const
{
    return d->translatedKey;
}

const QVariant& KFileMetaInfoItem::value() const
{
    return d->value;
}

QVariant::Type KFileMetaInfoItem::type() const
{
    return d->value.type();
}

bool KFileMetaInfoItem::isModified() const
{
    return d->dirty;
}

const QString& KFileMetaInfoItem::prefix() const
{
    return d->prefix;
}

const QString& KFileMetaInfoItem::suffix() const
{
    return d->suffix;
}

bool KFileMetaInfoItem::isEditable() const
{
    return d->editable;
}

bool KFileMetaInfoItem::isValid() const
{
    return d != &Data::null;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

// this is one object per file

KFileMetaInfo::KFileMetaInfo( const QString& path, const QString& mimeType )
    : d(new Data( path ) )
{
    kdDebug(7033) << "KFileMetaInfo( const QString& path )\n";

    QString mT;
    if (!mimeType)
        mT = KMimeType::findByURL(path, 0, true )->name();
    else 
        mT = mimeType;

    d->mimetype = mT;
    
    Internal item(*this);
    
    KFilePlugin* p = plugin(); if (p) p->readInfo( item );
    
    *this = item;
    
}

KFileMetaInfo::KFileMetaInfo( const KFileMetaInfo& original )
{
//    kdDebug(7033) << "KFileMetaInfo( const KFileMetaInfo& original)\n";
    *this = original;
    // d gets copied by value, i.e. the pointer is shared!
    if (d!=&Data::null) d->ref();
}

KFileMetaInfo::KFileMetaInfo()
{
    d = &Data::null;
}

KFileMetaInfo::~KFileMetaInfo()
{
//    if (d!=&Data::null) kdDebug(7033) << "KFileMetaInfo()\n";
    if ( d != &Data::null && d->deref() )
       delete d;
}

KFileMetaInfoItem & KFileMetaInfo::addItem( const QString& key,
                                            const QVariant& value )
{
    KFileMetaInfoItem item;
    d->items.insert(key, item);
    return d->items[key];
}

const KFileMetaInfo& KFileMetaInfo::operator= (const KFileMetaInfo& info )
{
    d = info.d;
    if (d != &Data::null) d->ref();
//    kdDebug(7033) << "info::operator=\n";
    return *this;
}


const QStringList KFileMetaInfo::preferredKeys() const
{
    QMapIterator<QString, KFileMetaInfoItem> it;
    QStringList list;
    
    kdDebug(7033) << "preferredKeys(), we have " << d->preferredKeys.size() << endl;
    kdDebug(7033) << "preferredKeys(), and " << d->items.size() << endl;

    if (d == &Data::null) kdWarning(7033) << "s.th. tries to get the prefferedKeys of an invalid metainfo object\n";
    
    for (it = d->items.begin(); it!=d->items.end(); ++it)
    {
        list.append(it.data().key());
    }

   // now move them up
    QStringList::Iterator all;
    QStringList::Iterator pref;
    
    // iterating backwards through a QStringList isn't very comfortable
    pref=d->preferredKeys.end();
    do
    {
        pref--;
        all = list.find(*pref);
        if (all != list.end())
        {
            QString tmp = *all;
            list.remove(all);
            list.prepend(tmp);
        }
    }
    while (pref!=d->preferredKeys.begin());
    
    return list;
}  

bool KFileMetaInfo::applyChanges()
{
    bool doit = false;

    kdDebug(7033) << "KFileMetaInfo::applyChanges()\n";
    
    // look up if we need to write to the file
    QMapIterator<QString, KFileMetaInfoItem> it;
    for (it = d->items.begin(); it!=d->items.end(); ++it)
    {
        if (it.data().isModified())
        {
            doit = true;
            break;
        }
    }
    if (!doit) 
    {
        kdDebug(7033) << "Don't need to write, nothing changed\n";
        return true;
    }
  
    KFilePlugin* p = plugin();
    if (!p) return false;
    
    kdDebug(7033) << "Ok, trying to write the info\n";
    
    return p->writeInfo(*this);
}

QValidator * KFileMetaInfo::createValidator( const QString& key,
                                                   QObject *parent,
                                                   const char *name ) const
{
    KFilePlugin* p = plugin();
    if (p) p->createValidator( operator[](key), parent, name );
}

KFilePlugin * const KFileMetaInfo::plugin() const
{
    KFileMetaInfoProvider* prov = KFileMetaInfoProvider::self();
    const KFilePlugin* plugin = prov->plugin( d->mimetype );
}

KFileMetaInfo::Data KFileMetaInfo::Data::null(QString::null);

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

KFilePlugin::KFilePlugin( QObject *parent, const char *name,
                          const QStringList& preferredItems)
    : QObject( parent, name )
{
    kdDebug(7033) << "loaded a plugin for " << name << endl;
    m_preferred = preferredItems;
}

KFilePlugin::~KFilePlugin()
{
    kdDebug(7033) << "unloaded a plugin for " << name() << endl;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


KFileMetaInfoProvider * KFileMetaInfoProvider::s_self = 0;
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
    s_self = 0;
    sd.setObject( 0 );
}


KFileMetaInfo KFileMetaInfoProvider::metaInfo( const QString& path )
{
    KURL url;
    url.setPath( path );
    return metaInfo( path, KMimeType::findByURL(path, 0, true )->name());
}

KFileMetaInfo KFileMetaInfoProvider::metaInfo( const QString& path,
                                                 const QString& mimeType )
{
    KFilePlugin * p = plugin( mimeType );

    if ( !p )
    {
        kdDebug(7033) << "no plugin found\n";
        return KFileMetaInfo();
    }
    
    kdWarning(7033) << "!!!!!! PROVIDER USED !!!!!!\n";
    
    KFileMetaInfo item(path, mimeType);
    return item;
}

KFilePlugin * KFileMetaInfoProvider::plugin(const QString& mimeType)
{
    KFilePlugin *p = m_plugins.find( mimeType );
    
    kdDebug(7033) << "mimetype is " << mimeType << endl;

    if ( !p )
    {
        kdDebug(7033) << "need to look for a plugin to load\n";

        KService::Ptr service =
            KServiceTypeProfile::preferredService( mimeType, "KFilePlugin");

        if ( !service || !service->isValid() )
        {
            kdDebug(7033) << "no valid service found\n";
            return 0;
        }
  
        // ### this needs fixing
        QVariant preferredItems = service->property("PreferredItems");
        
        p = KParts::ComponentFactory::createInstanceFromService<KFilePlugin>
                 ( service, this, mimeType.local8Bit(),
                   preferredItems.toStringList() );
        
        if (!p)
        {
            kdWarning(7033) << "error loading the plugin\n";
            return 0;
        }

        p->setMimeType( mimeType );
        kdDebug(7033) << "found a plugin\n";
        m_plugins.insert( mimeType, p );

    }
    else
        kdDebug(7033) << "plugin already loaded\n";
      
    kdDebug(7033) << "currently loaded plugins:\n";
    
    QDictIterator<KFilePlugin> it( m_plugins );
    for( ; it.current(); ++it )
        kdDebug(7033) << it.currentKey() << ": " << it.current()->className() << endl;

    return p;
}

#include "kfilemetainfo.moc"
