/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2001-2002 Rolf Maguns <ramagnus@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation version 2.0.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 *  $Id$
 */

#include <qshared.h>

#include <kstaticdeleter.h>
#include <kparts/componentfactory.h>
#include <kuserprofile.h>
#include <kdebug.h>
#include <kmimetype.h>
#include <kdatastream.h> // needed for serialization of bool

#include "kfilemetainfo.h"

// shared data of a KFileMetaInfoItem
class KFileMetaInfoItem::Data : public QShared
{
public:
    Data( const QString& _key, const QString& _translatedKey,
          const QVariant& _value, const QString& _prefix,
          const QString& _suffix, const bool _editable,
          const int _hint, const int _unit, const QString& _group )
        : QShared(),
          key( _key ),
          translatedKey( _translatedKey ),
          prefix( _prefix ),
          suffix( _suffix ),
          value( _value ),
          dirty( false ),
          editable( _editable ),
          hint( _hint ),
          unit( _unit ),
          group( _group )
    {}
    
    // wee use this one for the streaming operators
    Data() {};

    QString             key;
    QString             translatedKey;

    QString             prefix;
    QString             suffix;

    QVariant            value;

    bool                dirty    :1;
    bool                editable :1;
    
    int                 hint;
    int                 unit;
    QString             group;

    static Data* null;
    static Data* makeNull();
};

//this is our null data
KFileMetaInfoItem::Data* KFileMetaInfoItem::Data::null = 0L;
static KStaticDeleter<KFileMetaInfoItem::Data> sd_KFileMetaInfoItemData;

KFileMetaInfoItem::Data* KFileMetaInfoItem::Data::makeNull()
{
    if (!null)
        // We deliberately do not reset "null" after it has been destroyed!
        // Otherwise we will run into problems later in ~KFileMetaInfoItem 
        // where the d-pointer is compared against null.
        null = sd_KFileMetaInfoItemData.setObject( 
                new KFileMetaInfoItem::Data(QString::null, QString::null, QVariant(),
                                            QString::null, QString::null, false,
                                            Default, NoUnit, QString::null) );
    return null;
}

KFileMetaInfoItem::KFileMetaInfoItem( const QString& key,
                                      const QString& translatedKey,
                                      const QVariant& value,
                                      bool editable,
                                      const QString& prefix,
                                      const QString& suffix,
                                      const int hint,
                                      const int unit,
                                      const QString& group )
    : d( new Data(key, translatedKey, value, prefix, suffix, editable,
                  hint, unit, group))
{
}

KFileMetaInfoItem::KFileMetaInfoItem( const KFileMetaInfoItem& item )
{
    // operator= does everything that's neccessary
    d = Data::makeNull();
    *this = item;
}

KFileMetaInfoItem::KFileMetaInfoItem()
{
    d = Data::makeNull();
}

KFileMetaInfoItem::~KFileMetaInfoItem()
{
    deref();
}

const KFileMetaInfoItem& KFileMetaInfoItem::operator=
                                              (const KFileMetaInfoItem & item )
{
    if (d != item.d)
    {
        // first deref the old one
        deref();
        d = item.d;
        // and now ref the new one
        ref();
    }

    return *this;
}

bool KFileMetaInfoItem::setValue( const QVariant& value )
{
    kdDebug(7033) << "KFileMetaInfoItem::setValue() of item " << d->key << endl;
//    kdDebug(7033) << "isEditable: " << (d->editable ? "yes" : "no") << endl;
//    kdDebug(7033) << "type: " << value.typeName() << " and " << d->value.typeName()
//              << endl;
    
    // We don't call makeNull here since it isn't necassery:
    // If d is equal to null it means that null is initialized already.
    // null is 0L when it hasn't been initialized and d is never 0L.
    if ( d == Data::null ) return false;
    
    if ( !d->editable ||
         (d->value.isValid() && value.type() != d->value.type()) &&
                                value.type() != QVariant::Invalid)
        return false;

    d->dirty = true;
    d->value = value;

    return true;
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

QString KFileMetaInfoItem::prefix() const
{
    return d->prefix;
}

QString KFileMetaInfoItem::suffix() const
{
    return d->suffix;
}

int KFileMetaInfoItem::hint() const
{
    return d->hint;
}

bool KFileMetaInfoItem::isEditable() const
{
    return d->editable;
}

bool KFileMetaInfoItem::isValid() const
{
    // We don't call makeNull here since it isn't necassery:
    // If d is equal to null it means that null is initialized already.
    // null is 0L when it hasn't been initialized and d is never 0L.
    return d != Data::null;
}

void KFileMetaInfoItem::ref()
{
    if (d != Data::null) d->ref();
}

void KFileMetaInfoItem::deref()
{
    // We don't call makeNull here since it isn't necassery:
    // If d is equal to null it means that null is initialized already.
    // null is 0L when it hasn't been initialized and d is never 0L.
    if ((d != Data::null) && d->deref())
    {
        kdDebug(7033) << "item " << d->key << " is finally deleted\n";
        delete d;
    }
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

// shared data of a KFileMetaInfo
class KFileMetaInfo::Data : public QShared
{
public:
    Data(const QString& path)
        : QShared(),
          path(path),
          supportsVariableKeys(false)
    {}

    // wee use this one for the streaming operators
    Data() {};

    QString                           path;
    QString                           mimetype;
    QStringList                       supportedKeys;
    QStringList                       preferredKeys;
    bool                              supportsVariableKeys :1;

    bool                              supportsVariableGroups :1;
    QStringList                       supportedGroups;
    QStringList                       groups;
    
    QMap<QString, KFileMetaInfoItem>  items;

    static Data* null;
    static Data* makeNull();

};

KFileMetaInfo::KFileMetaInfo( const QString& path, const QString& mimeType,
                              int)
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
    
}

KFileMetaInfo::KFileMetaInfo( const KFileMetaInfo& original )
{
    // operator= does everything that's neccessary
    d = Data::makeNull();
    *this = original;
}

KFileMetaInfo::KFileMetaInfo()
{
    d = Data::makeNull();
}

KFileMetaInfo::~KFileMetaInfo()
{
    deref();
}

KFileMetaInfoItem KFileMetaInfo::addItem( const QString& key,
                                          const QString&)
{
  kdDebug(7033) << "additem of " << key << endl;

    if ((d->supportsVariableKeys) ||
        (!d->supportsVariableKeys && d->supportedKeys.contains(key)))
    {
        // empty item, unchecked for now, will fix this after 3.0
        // this is a hackish last-minute fix, but makes Phalynx happy
        // because it makes noatun-metatag work as expected
        // this will be subject to massive changes after the release
        // and I swear I'll not be so slow this time ;)  (Rolf)
        KFileMetaInfoItem item(key, key, QVariant(), true);
        d->items.insert(key, item);
        return item;
    }      
    // if the key is not supported, return an invalid item
    return KFileMetaInfoItem();
}

const QStringList KFileMetaInfo::supportedGroups() const
{
    return d->supportedGroups;
}

const QStringList KFileMetaInfo::groups() const
{
    return d->groups;
}

bool KFileMetaInfo::addGroup( const QString& name )
{
    if (d->supportedGroups.contains(name))
    {
        d->groups.append(name);
        return true;
    }
    
    return false;
}  

const KFileMetaInfo& KFileMetaInfo::operator= (const KFileMetaInfo& info )
{
    if (d != info.d)
    {
        deref();
        // first deref the old one
        d = info.d;
        // and now ref the new one
        ref();
    }
    return *this;
}

bool KFileMetaInfo::isValid() const
{
    // We don't call makeNull here since it isn't necassery:
    // If d is equal to null it means that null is initialized already.
    // null is 0L when it hasn't been initialized and d is never 0L.
    return d != Data::null;
}

bool KFileMetaInfo::isEmpty() const
{
    return d->items.isEmpty();
}

// ### Rolf, can you add some comment, what this method does? 
// It copies all keys from the map into a QStringList and then
// sorts them in some strange way? What for?
//
// It puts the ones that are in d->preferredKeys (which comes from the
// .desktop file) to the beginning, so the preferred keys are first in the list.
// I now changed it according to bruggie's suggestion. He says it's easier to
// understand this way.

QStringList KFileMetaInfo::preferredKeys() const
{
    if (d == Data::makeNull()) kdWarning(7033) << "attempt to get the preferredKeys of an invalid metainfo object";

    QStringList list;
    QStringList newlist;
    
    // make a QStringList with all available keys
    QMapIterator<QString, KFileMetaInfoItem> it;
    for (it = d->items.begin(); it!=d->items.end(); ++it)
    {
        list.append(it.data().key());
    }

    QStringList::Iterator pref;
    QStringList::Iterator begin = d->preferredKeys.begin();
    QStringList::Iterator end   = d->preferredKeys.end();

    // move all keys from d->preferredKeys that are in our list to a new list
    for ( pref = begin; pref!=end; pref++ )
    {
        QStringList::Iterator item = list.find(*pref);
        if ( item != list.end() )
        {
             newlist.append( *item );
             list.remove(item);
        }
    }

    // now the old list only contains the non-preferred items, so we
    // iterate over it and add the remaining ones to newlist
    begin = list.begin();
    end = list.end();
    for ( pref = begin; pref!=end; pref++ )
    {
        newlist.append( *pref );
    }
    
    return newlist;
}  

QStringList KFileMetaInfo::keys(const QString& ) const
{
    if (d == Data::makeNull()) kdWarning(7033) << "attempt to get the keys of an invalid metainfo object";

    QStringList list;
    
    // make a QStringList with all available keys
    QMapIterator<QString, KFileMetaInfoItem> it;
    for (it = d->items.begin(); it!=d->items.end(); ++it)
    {
        list.append(it.data().key());
    }
    
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
                                                   const char *name,
                                                   const QString& group) const
{
    KFilePlugin* p = plugin();
    if (p) return p->createValidator( key, parent, name, group );
    return 0;
}

KFilePlugin * const KFileMetaInfo::plugin() const
{
    KFileMetaInfoProvider* prov = KFileMetaInfoProvider::self();
    return prov->plugin( d->mimetype );
}

QStringList KFileMetaInfo::supportedKeys(const QString&) const
{
//    if (group == QString::null)
      return d->supportedKeys;
#if 0  
    QStringList::Iterator it  = d->supportedKeys.begin();
    QStringList::Iterator end = d->supportedKeys.end();
    
    QStringList l;
    
    for (; it != end; it++)
    {
        if (item(*it, group).isValid()) l.append(*it);
    }

    return l;
#endif
}

bool KFileMetaInfo::supportsVariableKeys(const QString&) const
{
    return d->supportsVariableKeys;
}

bool KFileMetaInfo::contains( const QString& key ) const
{
    return d->items.contains(key);
}

KFileMetaInfoItem & KFileMetaInfo::item( const QString& key,
                                         const QString&) const
{
    KFileMetaInfoItem& i = d->items[key];
    return i;
}

KFileMetaInfoItem & KFileMetaInfo::item(const KFileMetaInfoItem::Hint hint,
                                        const QString&) const
{
    QMapIterator<QString, KFileMetaInfoItem> it;
    
    for (it = d->items.begin(); it!=d->items.end(); ++it)
    {
        if ((it.data().hint() | hint) == hint)
            return it.data();
    }
    
    return *it;
}

void KFileMetaInfo::ref()
{
    if (d != Data::null) d->ref();
 
}

void KFileMetaInfo::deref()
{
    // We don't call makeNull here since it isn't necassery:
    // If d is equal to null it means that null is initialized already.
    // null is 0L when it hasn't been initialized and d is never 0L.
    if ((d != Data::null) && d->deref())
    {
        kdDebug(7033) << "metainfo object for " << d->path << " is finally deleted\n";
        delete d;
    }

}


KFileMetaInfo::Data* KFileMetaInfo::Data::null = 0L;
static KStaticDeleter<KFileMetaInfo::Data> sd_KFileMetaInfoData;

KFileMetaInfo::Data* KFileMetaInfo::Data::makeNull()
{
    if (!null)
        // We deliberately do not reset "null" after it has been destroyed!
        // Otherwise we will run into problems later in ~KFileMetaInfoItem 
        // where the d-pointer is compared against null.
	null = sd_KFileMetaInfoData.setObject( new KFileMetaInfo::Data(QString::null) );
    return null;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

KFileMetaInfo::Internal::~Internal()
{
}

void KFileMetaInfo::Internal::setSupportedKeys(const QStringList& keys)
{
    if (isValid())
        d->supportedKeys = keys;
}

void KFileMetaInfo::Internal::setPreferredKeys(const QStringList& keys)
{
    if (isValid())
        d->preferredKeys = keys;
}

void KFileMetaInfo::Internal::setSupportsVariableKeys(bool b,
                                                      const QString&)
{
    if (isValid())
        d->supportsVariableKeys = b;
}

void KFileMetaInfo::Internal::insert( const KFileMetaInfoItem &item )
{
    //kdDebug(7033) << "insert\n";
    if (isValid()) {
        //kdDebug(7033) << "insert really " << item.key() << endl;
        d->items.insert( item.key(), item );
        //kdDebug(7033) << "inserted " << item.key() << endl;
    }
}

const QString& KFileMetaInfo::Internal::path() const
{
    return d->path;
}

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

QStringList KFileMetaInfoProvider::preferredItems( const QString& mimeType )
{
    KService::Ptr service =
        KServiceTypeProfile::preferredService( mimeType, "KFilePlugin");
    
    if ( !service || !service->isValid() )
    {
        kdDebug(7033) << "no valid service found\n";
        return QStringList();
    }
    return service->property("PreferredItems").toStringList();
}

/* serialization of a KFileMetaInfoItem:
   first a bool that says if the items is valid, and if yes,
   all the elements of the Data
*/
QDataStream& operator <<(QDataStream& s, const KFileMetaInfoItem& item )
{
    KFileMetaInfoItem::Data* d = item.d;

    // if the object is invalid, put only the bool in the stream
    bool isValid = item.isValid();
    s << isValid;
    if (isValid)
        s << d->key
          << d->translatedKey
          << d->prefix
          << d->suffix
          << d->value
          << d->dirty
          << d->editable
          << d->hint
          << d->unit
          << d->group;

    return s;
};


QDataStream& operator >>(QDataStream& s, KFileMetaInfoItem& item )
{
    bool isValid;
    s >> isValid;

    if (!isValid)
    {
        item = KFileMetaInfoItem();
        return s;
    }

    // we need a new object for our data
    item.deref();
    item.d = new KFileMetaInfoItem::Data();

    bool dirty, editable;
    s >> item.d->key
      >> item.d->translatedKey
      >> item.d->prefix
      >> item.d->suffix
      >> item.d->value
      >> dirty
      >> editable
      >> item.d->hint
      >> item.d->unit
      >> item.d->group;
      item.d->dirty = dirty;
      item.d->editable = editable;

    return s;
}

QDataStream& operator <<(QDataStream& s, const KFileMetaInfo& info )
{
    KFileMetaInfo::Data* d = info.d;

    // if the object is invalid, put only the bool in the stream
    bool isValid = info.isValid();
    s << isValid;
    if (isValid)
    {
        s << d->path
          << d->mimetype
          << d->supportedKeys
          << d->preferredKeys
          << d->supportsVariableKeys
          << d->supportsVariableGroups
          << d->supportedGroups
          << d->groups
          << d->items;
    }    
    return s;
};


QDataStream& operator >>(QDataStream& s, KFileMetaInfo& info )
{
    bool isValid;
    s >> isValid;

    // if it's invalid, there is not much to do
    if (!isValid)
    {
        info = KFileMetaInfo();
        return s;
    }

    // we need a new object for our data
    info.deref();
    info.d = new KFileMetaInfo::Data();
    info.ref();
    
    bool supportsVariableKeys, supportsVariableGroups;

    s >> info.d->path
      >> info.d->mimetype
      >> info.d->supportedKeys
      >> info.d->preferredKeys
      >> supportsVariableKeys
      >> supportsVariableGroups
      >> info.d->supportedGroups
      >> info.d->groups
      >> info.d->items;
      info.d->supportsVariableKeys = supportsVariableKeys;
      info.d->supportsVariableGroups = supportsVariableGroups;
      
    return s;
}

#include "kfilemetainfo.moc"
