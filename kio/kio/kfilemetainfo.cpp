/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2001-2002 Rolf Magnus <ramagnus@kde.org>
 *  Copyright (C) 2001-2002 Carsten Pfeiffer <pfeiffer@kde.org>
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
#include <qdict.h>

#include <kstaticdeleter.h>
#include <kparts/componentfactory.h>
#include <kuserprofile.h>
#include <kdebug.h>
#include <kmimetype.h>
#include <kdatastream.h> // needed for serialization of bool
#include <klocale.h>

#include "kfilemetainfo.h"

// shared data of a KFileMetaInfoItem
class KFileMetaInfoItem::Data : public QShared
{
public:
    Data( const KFileMimeTypeInfo::ItemInfo* mti, const QString& _key,
          const QVariant& _value )
        : QShared(),
          mimeTypeInfo( mti ),
          key( _key ),
          value( _value )
    {}

    // wee use this one for the streaming operators
    Data() {};

    const KFileMimeTypeInfo::ItemInfo*  mimeTypeInfo;
    // mimeTypeInfo has the key, too, but only for non-variable ones
    QString                             key;
    QVariant                            value;
    bool                                dirty    :1;
    bool                                added    :1;
    bool                                removed  :1;

    static Data* null;
    static Data* makeNull();
};

//this is our null data
KFileMetaInfoItem::Data* KFileMetaInfoItem::Data::null = 0L;
static KStaticDeleter<KFileMetaInfoItem::Data> sd_KFileMetaInfoItemData;

KFileMetaInfoItem::Data* KFileMetaInfoItem::Data::makeNull()
{
    if (!null)
    {
        // We deliberately do not reset "null" after it has been destroyed!
        // Otherwise we will run into problems later in ~KFileMetaInfoItem
        // where the d-pointer is compared against null.

        // ### fix (small memory leak)
        KFileMimeTypeInfo::ItemInfo* info = new KFileMimeTypeInfo::ItemInfo();
        Data* d = new Data(info, QString::null, QVariant());
        null = sd_KFileMetaInfoItemData.setObject( d );
    }
    return null;
}

KFileMetaInfoItem::KFileMetaInfoItem( const KFileMimeTypeInfo::ItemInfo* mti,
                                      const QString& key, const QVariant& value )
    : d( new Data( mti, key, value ) )
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
    // We don't call makeNull here since it isn't necassery, see deref()
    if ( d == Data::null ) return false;
    
    if ( ! (d->mimeTypeInfo->attributes() & KFileMimeTypeInfo::Modifiable ) ||
         ! (value.canCast(d->mimeTypeInfo->type())))
    {
        kdDebug(7033) << "setting the value of " << key() << "failed\n";
        return false;
    }

    kdDebug(7033) << key() << ".setValue()\n";

    d->dirty = true;
    d->value = value;
    // If we don't cast (and test for canCast in the above if), QVariant is
    // very picky about types (e.g. QString vs. QCString or int vs. uint)
    d->value.cast(d->mimeTypeInfo->type());

    return true;
}

bool KFileMetaInfoItem::isRemoved() const
{
    return d->removed;
}

QString KFileMetaInfoItem::key() const
{
    return d->key;
}

QString KFileMetaInfoItem::translatedKey() const
{
    // are we a variable key?
    if (d->mimeTypeInfo->key().isNull())
    {
        // then try if we have luck with i18n()
        return i18n(d->key.utf8());
    }

    return d->mimeTypeInfo->translatedKey();
}

const QVariant& KFileMetaInfoItem::value() const
{
    return d->value;
}

QString KFileMetaInfoItem::string( bool mangle ) const
{
    QString s;

    switch (d->value.type())
    {
        case QVariant::Invalid :
            return "---";

        case QVariant::Bool :
            s = d->value.toBool() ? i18n("Yes") : i18n("No");
            break;

        case QVariant::Int :
            if (d->mimeTypeInfo->unit() == KFileMimeTypeInfo::Seconds)
            {
              int seconds = d->value.toInt() % 60;
              int minutes = d->value.toInt() / 60 % 60;
              int hours   = d->value.toInt() / 3600;
              s = hours ? QString().sprintf("%d:%02d:%02d",hours, minutes, seconds)
                        : QString().sprintf("%0d:%02d", minutes, seconds);
            } else
                s = KGlobal::locale()->formatNumber( d->value.toInt() , 0);
            break;

        case QVariant::UInt :
            s = KGlobal::locale()->formatNumber( d->value.toUInt() , 0);
            break;

        case QVariant::Double :
            s = KGlobal::locale()->formatNumber( d->value.toDouble(), 3);
            break;

        case QVariant::Date :
            s = KGlobal::locale()->formatDate( d->value.toDate(), true );
            break;

        case QVariant::Time :
            s = KGlobal::locale()->formatTime( d->value.toTime(), true );
            break;

        case QVariant::DateTime :
            s = KGlobal::locale()->formatDateTime( d->value.toDateTime(),
                                                   true, true );
            break;

        case QVariant::Size :
            s = QString("%1x%2").arg(d->value.toSize().width())
                                .arg(d->value.toSize().height());
            break;

        case QVariant::Point :
            s = QString("%1/%2").arg(d->value.toSize().width())
                                .arg(d->value.toSize().height());
            break;

        default:
            s = d->value.toString();
    }

    if (mangle && !s.isNull())
    {
        s.prepend(prefix());
        s.append(suffix());
    }
    return s;
}

QVariant::Type KFileMetaInfoItem::type() const
{
    return d->mimeTypeInfo->type();
}

bool KFileMetaInfoItem::isModified() const
{
    return d->dirty;
}

QString KFileMetaInfoItem::prefix() const
{
    return d->mimeTypeInfo->prefix();
}

QString KFileMetaInfoItem::suffix() const
{
    return d->mimeTypeInfo->suffix();
}

uint KFileMetaInfoItem::hint() const
{
    return d->mimeTypeInfo->hint();
}

uint KFileMetaInfoItem::attributes() const
{
    return d->mimeTypeInfo->attributes();
}

bool KFileMetaInfoItem::isEditable() const
{
    return d->mimeTypeInfo->attributes() & KFileMimeTypeInfo::Modifiable;
}

bool KFileMetaInfoItem::isValid() const
{
    // We don't call makeNull here since it isn't necassery:
    // If d is equal to null it means that null is initialized already.
    // null is 0L when it hasn't been initialized and d is never 0L.
    return d != Data::null;
}

void KFileMetaInfoItem::setAdded()
{
    d->added = true;
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
        kdDebug(7033) << "item " << d->key
                      << " is finally deleted\n";
        delete d;
    }
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

// shared data of a KFileMetaInfo
class KFileMetaInfo::Data : public QShared
{
public:
    Data(const QString& _path, uint _what)
        : QShared(),
          path(_path),
          what(_what)
    {}

    // wee use this one for the streaming operators
    Data() {};

    QString                           path;
    uint                              what;
    QMap<QString, KFileMetaInfoGroup> groups;
    const KFileMimeTypeInfo*          mimeTypeInfo;
    QStringList                       removedGroups;

    static Data* null;
    static Data* makeNull();

};

KFileMetaInfo::KFileMetaInfo( const QString& path, const QString& mimeType,
                              uint what )
    : d(new Data( path, what ) )
{
    kdDebug(7033) << "KFileMetaInfo( " << path << ", " << mimeType << ", what )\n";

    QString mT;
    if (mimeType.isEmpty())
        mT = KMimeType::findByURL(path, 0, true )->name();
    else
        mT = mimeType;

    // let's "share our property"
    KFileMetaInfo item(*this);

    d->mimeTypeInfo = KFileMetaInfoProvider::self()->mimeTypeInfo(mT);
    if ( d->mimeTypeInfo )
    {
        kdDebug(7033) << "Found mimetype info for " << mT << endl;
        KFilePlugin *p = plugin();
        if (p && !p->readInfo( item, what))
            *this=KFileMetaInfo();
    }
    else 
    {
        kdDebug(7033) << "No mimetype info for " << mimeType << endl;
        d = Data::makeNull();
    }
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

QStringList KFileMetaInfo::supportedGroups() const
{
    return d->mimeTypeInfo->supportedGroups();
}

QStringList KFileMetaInfo::supportedKeys() const
{
    return d->mimeTypeInfo->supportedKeys();
}

QStringList KFileMetaInfo::groups() const
{
    QStringList list;
    QMapConstIterator<QString, KFileMetaInfoGroup> it = d->groups.begin();
    for ( ; it != d->groups.end(); ++it )
        list += (*it).name();

    return list;
}

QStringList KFileMetaInfo::editableGroups() const
{
    QStringList list;
    QStringList supported = supportedGroups();
    QStringList::ConstIterator it = supported.begin();
    for ( ; it != supported.end(); ++it ) {
        const KFileMimeTypeInfo::GroupInfo * groupInfo = d->mimeTypeInfo->groupInfo( *it );
        if ( groupInfo && groupInfo->attributes() & 
             (KFileMimeTypeInfo::Addable | KFileMimeTypeInfo::Removable) )
            list.append( *it );
    }

    return list;
}

QStringList KFileMetaInfo::preferredGroups() const
{
    QStringList list = groups();
    QStringList newlist;
    QStringList preferred = d->mimeTypeInfo->preferredGroups();
    QStringList::Iterator pref;

    // move all keys from the preferred groups that are in our list to a new list
    for ( pref = preferred.begin(); pref != preferred.end(); pref++ )
    {
        QStringList::Iterator group = list.find(*pref);
        if ( group != list.end() )
        {
             newlist.append( *group );
             list.remove(group);
        }
    }

    // now the old list only contains the non-preferred items, so we
    // add the remaining ones to newlist
    newlist += list;

    return newlist;
}

QStringList KFileMetaInfo::preferredKeys() const
{
    QStringList newlist;

    QStringList list = preferredGroups();
    for (QStringList::Iterator git = list.begin(); git != list.end(); ++git)
    {
        newlist += d->groups[*git].preferredKeys();
    }

    return newlist;
}

KFileMetaInfoGroup KFileMetaInfo::group(const QString& key) const
{
    QMapIterator<QString,KFileMetaInfoGroup> it = d->groups.find( key );
    if ( it != d->groups.end() )
        return it.data();
    else
        return KFileMetaInfoGroup();
}

bool KFileMetaInfo::addGroup( const QString& name )
{
    if ( d->mimeTypeInfo->supportedGroups().contains(name) &&
         ! d->groups.contains(name) )
    {
        KFileMetaInfoGroup group( name, d->mimeTypeInfo );

        // add all the items that can't be added by the user later
        const KFileMimeTypeInfo::GroupInfo* ginfo = d->mimeTypeInfo->groupInfo(name);
        Q_ASSERT(ginfo);
        if (!ginfo) return false;

        QStringList keys = ginfo->supportedKeys();
        for (QStringList::Iterator it = keys.begin(); it != keys.end(); ++it)
        {
            const KFileMimeTypeInfo::ItemInfo* iteminfo = ginfo->itemInfo(*it);
            Q_ASSERT(ginfo);
            if (!iteminfo) return false;
            
            if ( !(iteminfo->attributes() & KFileMimeTypeInfo::Addable) &&
                  (iteminfo->attributes() & KFileMimeTypeInfo::Modifiable))
            {
                // append it now or never
                group.appendItem(iteminfo->key(), QVariant());
            }
              
        }

        d->groups.insert(name, group);
        group.setAdded();
        return true;
    }

    return false;
}

bool KFileMetaInfo::removeGroup( const QString& name )
{
    QMapIterator<QString, KFileMetaInfoGroup> it = d->groups.find(name);
    if ( (it==d->groups.end()) ||
        !((*it).attributes() & KFileMimeTypeInfo::Removable))
        return false;

    d->groups.remove(it);
    d->removedGroups.append(name);
    return true;
}

QStringList KFileMetaInfo::removedGroups()
{
    return d->removedGroups;
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
    // We don't call makeNull here since it isn't necassery, see deref()
    return d != Data::null;
}

bool KFileMetaInfo::isEmpty() const
{
    for (QMapIterator<QString, KFileMetaInfoGroup> it = d->groups.begin();
         it!=d->groups.end(); ++it)
        if (!(*it).isEmpty())
            return false;
    return true;
}

bool KFileMetaInfo::applyChanges()
{
    bool doit = false;

    kdDebug(7033) << "KFileMetaInfo::applyChanges()\n";

    // look up if we need to write to the file
    QMapConstIterator<QString, KFileMetaInfoGroup> it;
    for (it = d->groups.begin(); it!=d->groups.end(); ++it)
    {
        QStringList keys = it.data().keys();
        for (QStringList::Iterator it2 = keys.begin(); it2!=keys.end(); ++it)
        {
            if ((*it)[*it2].isModified());
            {
                doit = true;
                break;
            }
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

KFilePlugin * const KFileMetaInfo::plugin() const
{
    KFileMetaInfoProvider* prov = KFileMetaInfoProvider::self();
    return prov->plugin( d->mimeTypeInfo->mimeType() );
}

QString KFileMetaInfo::mimeType() const
{
    return d->mimeTypeInfo->mimeType();
}

bool KFileMetaInfo::contains(const QString& key) const
{
    QStringList glist = groups();
    for (QStringList::Iterator it = glist.begin(); it != glist.end(); ++it)
    {
        KFileMetaInfoGroup g = d->groups[*it];
        if (g.contains(key)) return true;
    }
    return false;
}

bool KFileMetaInfo::containsGroup(const QString& key) const
{
    return groups().contains(key);
}

KFileMetaInfoItem KFileMetaInfo::item( const QString& key) const
{
    QStringList groups = preferredGroups();
    for (QStringList::Iterator it = groups.begin(); it != groups.end(); ++it)
    {
        KFileMetaInfoItem i = d->groups[*it][key];
        if (i.isValid()) return i;
    }
    return KFileMetaInfoItem();
}

KFileMetaInfoItem KFileMetaInfo::item(const KFileMetaInfoItem::Hint hint) const
{
    QStringList groups = preferredGroups();
    QStringList::ConstIterator it;
    for (it = groups.begin(); it != groups.end(); ++it)
    {
        KFileMetaInfoItem i = d->groups[*it].item(hint);
        if (i.isValid()) return i;
    }
    return KFileMetaInfoItem();
}

KFileMetaInfoItem KFileMetaInfo::saveItem( const QString& key,
                                           const QString& preferredGroup,
                                           bool createGroup )
{
    // try the preferred groups first
    if ( !preferredGroup.isEmpty() ) {
        QMapIterator<QString,KFileMetaInfoGroup> it = 
            d->groups.find( preferredGroup );

        // try to create the preferred group, if necessary
        if ( it == d->groups.end() && createGroup ) {
            const KFileMimeTypeInfo::GroupInfo *groupInfo = 
                d->mimeTypeInfo->groupInfo( preferredGroup );
            if ( groupInfo && groupInfo->supportedKeys().contains( key ) ) {
                if ( addGroup( preferredGroup ) )
                    it = d->groups.find( preferredGroup );
            }
        }
        
        if ( it != d->groups.end() ) {
            KFileMetaInfoItem item = it.data().addItem( key );
            if ( item.isValid() )
                return item;
        }
    }
    
    QStringList groups = preferredGroups();

    KFileMetaInfoItem item;

    QStringList::ConstIterator groupIt = groups.begin();
    for ( ; groupIt != groups.end(); ++groupIt )
    {
        QMapIterator<QString,KFileMetaInfoGroup> it = d->groups.find( *groupIt );
        if ( it != d->groups.end() ) 
        {
            KFileMetaInfoGroup group = it.data();
            item = findEditableItem( group, key );
            if ( item.isValid() )
                return item;
        }
        else // not existant -- try to create the group
        {
            const KFileMimeTypeInfo::GroupInfo *groupInfo = 
                d->mimeTypeInfo->groupInfo( *groupIt );
            if ( groupInfo && groupInfo->supportedKeys().contains( key ) ) 
            {
                if ( addGroup( *groupIt ) )
                {
                    KFileMetaInfoGroup group = d->groups[*groupIt];
                    KFileMetaInfoItem item = group.addItem( key );
                    if ( item.isValid() )
                        return item;
//                     else ### add when removeGroup() is implemented :)
//                         removeGroup( *groupIt ); // couldn't add item -> remove
                }
            }
        }
    }

    // finally check for variable items
    
    return item;
}

KFileMetaInfoItem KFileMetaInfo::findEditableItem( KFileMetaInfoGroup& group, 
                                                   const QString& key )
{
    KFileMetaInfoItem item = group.addItem( key );
    if ( item.isValid() && item.isEditable() )
         return item;
    
    if ( (d->mimeTypeInfo->groupInfo( group.name() )->attributes() & KFileMimeTypeInfo::Addable) )
        return item;

    return KFileMetaInfoItem();
}

KFileMetaInfoGroup KFileMetaInfo::appendGroup(const QString& name)
{
    if ( d->mimeTypeInfo->supportedGroups().contains(name) &&
         ! d->groups.contains(name) )
    {
        KFileMetaInfoGroup group( name, d->mimeTypeInfo );
        d->groups.insert(name, group);
        return group;
    }

    else {
        kdWarning(7033) << "Someone's trying to add a KFileMetaInfoGroup which is not supported or already existing: " << name << endl;
        return KFileMetaInfoGroup();
    }
}

QString KFileMetaInfo::path() const
{
    return d->path;
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
	null = sd_KFileMetaInfoData.setObject( new KFileMetaInfo::Data(QString::null, 0) );
    return null;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

KFilePlugin::KFilePlugin( QObject *parent, const char *name,
                          const QStringList& /*args*/)
    : QObject( parent, name )
{
    kdDebug(7033) << "loaded a plugin for " << name << endl;
}

KFilePlugin::~KFilePlugin()
{
    kdDebug(7033) << "unloaded a plugin for " << name() << endl;
}

KFileMimeTypeInfo * KFilePlugin::addMimeTypeInfo( const QString& mimeType )
{
    KFileMimeTypeInfo* info;

    info = KFileMetaInfoProvider::self()-> addMimeTypeInfo( mimeType );
    return info;
}

void KFilePlugin::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }


KFileMimeTypeInfo::GroupInfo*  KFilePlugin::addGroupInfo(KFileMimeTypeInfo* info,
                  const QString& key, const QString& translatedKey) const
{
    return info->addGroupInfo(key, translatedKey);
}

void KFilePlugin::setAttributes(KFileMimeTypeInfo::GroupInfo* gi, uint attr) const
{
    gi->m_attr = attr;
}

void KFilePlugin::addVariableInfo(KFileMimeTypeInfo::GroupInfo* gi,
                                  QVariant::Type type, uint attr) const
{
    gi->addVariableInfo(type, attr);
}

KFileMimeTypeInfo::ItemInfo* KFilePlugin::addItemInfo(KFileMimeTypeInfo::GroupInfo* gi,
                                                     const QString& key,
                                                     const QString& translatedKey,
                                                     QVariant::Type type)
{
    return gi->addItemInfo(key, translatedKey, type);
}

void KFilePlugin::setAttributes(KFileMimeTypeInfo::ItemInfo* item, uint attr)
{
    item->m_attr = attr;
}

void KFilePlugin::setHint(KFileMimeTypeInfo::ItemInfo* item, uint hint)
{
    item->m_hint = hint;
}

void KFilePlugin::setUnit(KFileMimeTypeInfo::ItemInfo* item, uint unit)
{
    item->m_unit = unit;
}

void KFilePlugin::setPrefix(KFileMimeTypeInfo::ItemInfo* item, const QString& prefix)
{
    item->m_prefix = prefix;
}

void KFilePlugin::setSuffix(KFileMimeTypeInfo::ItemInfo* item, const QString& suffix)
{
    item->m_suffix = suffix;
}

KFileMetaInfoGroup KFilePlugin::appendGroup(KFileMetaInfo& info, const QString& key)
{
    return info.appendGroup(key);
}

void KFilePlugin::appendItem(KFileMetaInfoGroup& group, const QString& key, QVariant value)
{
    group.appendItem(key, value);
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
    m_mimeTypeDict.setAutoDelete( true );
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

        p = KParts::ComponentFactory::createInstanceFromService<KFilePlugin>
                 ( service, this, mimeType.local8Bit() );

        if (!p)
        {
            kdWarning(7033) << "error loading the plugin\n";
            return 0;
        }

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

QStringList KFileMetaInfoProvider::preferredKeys( const QString& mimeType ) const
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

QStringList KFileMetaInfoProvider::preferredGroups( const QString& mimeType ) const
{
    KService::Ptr service =
        KServiceTypeProfile::preferredService( mimeType, "KFilePlugin");

    if ( !service || !service->isValid() )
    {
        kdDebug(7033) << "no valid service found\n";
        return QStringList();
    }
    return service->property("PreferredGroups").toStringList();
}

const KFileMimeTypeInfo * KFileMetaInfoProvider::mimeTypeInfo( const QString& mimeType )
{
    KFileMimeTypeInfo *info = m_mimeTypeDict.find( mimeType );
    if ( !info ) {
        // create the plugin (adds the mimeTypeInfo, if possible)
        KFilePlugin *p = plugin( mimeType );
        if ( p )
            info = m_mimeTypeDict.find( mimeType );
    }

    return info;
}

KFileMimeTypeInfo * KFileMetaInfoProvider::addMimeTypeInfo(
        const QString& mimeType )
{
    KFileMimeTypeInfo *info = m_mimeTypeDict.find( mimeType );
    if ( !info )
    {
        info = new KFileMimeTypeInfo( mimeType );
        m_mimeTypeDict.replace( mimeType, info );
    }

    info->m_preferredKeys    = preferredKeys( mimeType );
    info->m_preferredGroups  = preferredGroups( mimeType );

    return info;
}


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


// shared data of a KFileMetaInfoGroup
class KFileMetaInfoGroup::Data : public QShared
{
public:
    Data(const QString& _name)
        : QShared(),
          name(_name)
    {}

    // wee use this one for the streaming operators
    Data() {};

    QString                             name;
    QMap<QString, KFileMetaInfoItem>    items;
    const KFileMimeTypeInfo*            mimeTypeInfo;
    QStringList                         removedItems;
    bool                                dirty   :1;
    bool                                added   :1;
    bool                                removed :1;

    static Data* null;
    static Data* makeNull();

};

KFileMetaInfoGroup::KFileMetaInfoGroup( const QString& name,
                                        const KFileMimeTypeInfo* info )
    : d(new Data( name ) )
{
      d->mimeTypeInfo = info;
}

KFileMetaInfoGroup::KFileMetaInfoGroup( const KFileMetaInfoGroup& original )
{
    // operator= does everything that's neccessary
    d = Data::makeNull();
    *this = original;
}

KFileMetaInfoGroup::KFileMetaInfoGroup()
{
    d = Data::makeNull();
}

KFileMetaInfoGroup::~KFileMetaInfoGroup()
{
    deref();
}

const KFileMetaInfoGroup& KFileMetaInfoGroup::operator= (const KFileMetaInfoGroup& info )
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

bool KFileMetaInfoGroup::isValid() const
{
    // We don't call makeNull here since it isn't necassery, see deref()
    return d != Data::null;
}

bool KFileMetaInfoGroup::isEmpty() const
{
    return d->items.isEmpty();
}

QStringList KFileMetaInfoGroup::preferredKeys() const
{
    if (d == Data::makeNull())
          kdWarning(7033) << "attempt to get the preferredKeys of "
                              "an invalid metainfo group";

    QStringList list = keys();
    QStringList newlist;
    QStringList preferredKeys = d->mimeTypeInfo->preferredKeys();
    QStringList::Iterator pref;
    QStringList::Iterator begin = preferredKeys.begin();
    QStringList::Iterator end   = preferredKeys.end();

    // move all keys from the preferred keys that are in our list to a new list
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
    // add the remaining ones to newlist
    newlist += list;

    return newlist;
}

QStringList KFileMetaInfoGroup::keys() const
{
    if (d == Data::makeNull())
        kdWarning(7033) << "attempt to get the keys of "
                           "an invalid metainfo group";

    QStringList list;

    // make a QStringList with all available keys
    QMapConstIterator<QString, KFileMetaInfoItem> it;
    for (it = d->items.begin(); it!=d->items.end(); ++it)
    {
        list.append(it.data().key());
        kdDebug(7033) << "Item " << it.data().key() << endl;
    }
    return list;
}

QStringList KFileMetaInfoGroup::supportedKeys() const
{
      return d->mimeTypeInfo->groupInfo(d->name)->supportedKeys();
}

bool KFileMetaInfoGroup::supportsVariableKeys() const
{
      return d->mimeTypeInfo->groupInfo(d->name)->supportsVariableKeys();
}

bool KFileMetaInfoGroup::contains( const QString& key ) const
{
    return d->items.contains(key);
}

KFileMetaInfoItem KFileMetaInfoGroup::item( const QString& key) const
{
    QMapIterator<QString,KFileMetaInfoItem> it = d->items.find( key );
    if ( it != d->items.end() )
        return it.data();

    return KFileMetaInfoItem();
}

KFileMetaInfoItem KFileMetaInfoGroup::item(uint hint) const
{
    QMapIterator<QString, KFileMetaInfoItem> it;

    for (it = d->items.begin(); it!=d->items.end(); ++it)
    {
        if (it.data().hint() & hint) return it.data();
    }

    return KFileMetaInfoItem();
}

QString KFileMetaInfoGroup::name() const
{
    return d->name;
}

uint KFileMetaInfoGroup::attributes() const
{
    return d->mimeTypeInfo->groupInfo(d->name)->attributes();
}

void KFileMetaInfoGroup::setAdded()
{
    d->added = true;
}

void KFileMetaInfoGroup::ref()
{
    if (d != Data::null) d->ref();

}

void KFileMetaInfoGroup::deref()
{
    // We don't call makeNull here since it isn't necassery:
    // If d is equal to null it means that null is initialized already.
    // null is 0L when it hasn't been initialized and d is never 0L.
    if ((d != Data::null) && d->deref())
    {
        kdDebug(7033) << "metainfo group " << d->name
                      << " is finally deleted\n";
        delete d;
    }

}

KFileMetaInfoItem KFileMetaInfoGroup::addItem( const QString& key )
{
    QMapIterator<QString,KFileMetaInfoItem> it = d->items.find( key );
    if ( it != d->items.end() )
        return it.data();
    
    const KFileMimeTypeInfo::GroupInfo* ginfo = d->mimeTypeInfo->groupInfo(d->name);

    if ( !ginfo ) {
        Q_ASSERT( ginfo );
        return KFileMetaInfoItem();
    }

    const KFileMimeTypeInfo::ItemInfo* info = ginfo->itemInfo(key);

    if ( !info ) {
        Q_ASSERT( info );
        return KFileMetaInfoItem();
    }

    KFileMetaInfoItem item;

    if (info->isVariableItem())
        item = KFileMetaInfoItem(ginfo->variableItemInfo(), key, QVariant());
    else
        item = KFileMetaInfoItem(info, key, QVariant());

    d->items.insert(key, item);
    item.setAdded();           // mark as added
    d->dirty = true;           // mark ourself as dirty, too
    return item;
}

bool KFileMetaInfoGroup::removeItem( const QString& key )
{
    if (!isValid())
    {
          kdDebug(7033) << "trying to remove an item from an invalid group\n";
          return false;
    }
    
    QMapIterator<QString, KFileMetaInfoItem> it = d->items.find(key);
    if ( it==d->items.end() )
    {
          kdDebug(7033) << "trying to remove the non existant item " << key << "\n";
          return false;
    }
    
    if (!((*it).attributes() & KFileMimeTypeInfo::Removable))
    {
        kdDebug(7033) << "trying to remove a non removable item\n";
        return false;
    }
    
    d->items.remove(it);
    d->removedItems.append(key);
    return true;
}
  
QStringList KFileMetaInfoGroup::removedItems()
{
    return d->removedItems;
}

KFileMetaInfoItem KFileMetaInfoGroup::appendItem(const QString& key,
                                                 const QVariant& value)
{
    const KFileMimeTypeInfo::GroupInfo* ginfo = d->mimeTypeInfo->groupInfo(d->name);
    if ( !ginfo ) {
        kdWarning() << "Trying to append a Metadata item for a non-existant group:" << d->name << endl;
        return KFileMetaInfoItem();
    }
    const KFileMimeTypeInfo::ItemInfo* info = ginfo->itemInfo(key);
    if ( !info ) {
        kdWarning() << "Trying to append a Metadata item for an unknown key (no ItemInfo): " << key << endl;
        return KFileMetaInfoItem();
    }

    KFileMetaInfoItem item;

    if (info->key().isNull())
        item = KFileMetaInfoItem(ginfo->variableItemInfo(), key, value);
    else
        item = KFileMetaInfoItem(info, key, value);

    kdDebug(7033) << "KFileMetaInfogroup inserting a " << key << endl;

    d->items.insert(key, item);
    return item;
}

KFileMetaInfoGroup::Data* KFileMetaInfoGroup::Data::null = 0L;
static KStaticDeleter<KFileMetaInfoGroup::Data> sd_KFileMetaInfoGroupData;

KFileMetaInfoGroup::Data* KFileMetaInfoGroup::Data::makeNull()
{
    if (!null)
    {
        // We deliberately do not reset "null" after it has been destroyed!
        // Otherwise we will run into problems later in ~KFileMetaInfoItem
        // where the d-pointer is compared against null.
      
        // ### fix (small memory leak)
        KFileMimeTypeInfo* info = new KFileMimeTypeInfo();
        Data* d = new Data(QString::null);
        d->mimeTypeInfo = info;
        null = sd_KFileMetaInfoGroupData.setObject( d );
    }
    return null;
}


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

KFileMimeTypeInfo::KFileMimeTypeInfo( const QString& mimeType )
    : m_mimeType( mimeType )
{
    m_groups.setAutoDelete( true );
}

KFileMimeTypeInfo::~KFileMimeTypeInfo()
{
}

const KFileMimeTypeInfo::GroupInfo * KFileMimeTypeInfo::groupInfo( const QString& group ) const
{
    return m_groups.find( group );
}

KFileMimeTypeInfo::GroupInfo * KFileMimeTypeInfo::addGroupInfo(
                           const QString& name, const QString& translatedName )
{
    GroupInfo* group = new GroupInfo( name, translatedName );
    m_groups.insert(name, group);
    return group;
}

QStringList KFileMimeTypeInfo::supportedGroups() const
{
    QStringList list;
    QDictIterator<GroupInfo> it( m_groups );
    for ( ; it.current(); ++it )
        list.append( it.current()->name() );

    return list;
}

QStringList KFileMimeTypeInfo::translatedGroups() const
{
    QStringList list;
    QDictIterator<GroupInfo> it( m_groups );
    for ( ; it.current(); ++it )
        list.append( it.current()->translatedName() );

    return list;
}

QStringList KFileMimeTypeInfo::supportedKeys() const
{
    // not really efficient, but not those are not large lists, probably.
    // maybe cache the result?
    QStringList keys;
    QStringList::ConstIterator lit;
    QDictIterator<GroupInfo> it( m_groups );
    for ( ; it.current(); ++it ) { // need to nuke dupes
        QStringList list = it.current()->supportedKeys();
        for ( lit = list.begin(); lit != list.end(); ++lit ) {
            if ( keys.find( *lit ) == keys.end() )
                keys.append( *lit );
        }
    }

    return keys;
}

QValidator * KFileMimeTypeInfo::createValidator(const QString& group,
                                                const QString& key,
                                                QObject *parent,
                                                const char *name) const
{
    KFilePlugin* plugin = KFileMetaInfoProvider::self()->plugin(m_mimeType);
    if (plugin) return plugin->createValidator(mimeType(), group, key,
                                               parent, name);
    return 0;
}


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

KFileMimeTypeInfo::GroupInfo::GroupInfo( const QString& name,
                                         const QString& translatedName )
    : m_name( name ),
      m_translatedName( translatedName ),
      m_attr( 0 ),
      m_variableItemInfo( 0 )

{
    m_itemDict.setAutoDelete( true );
}

const KFileMimeTypeInfo::ItemInfo * KFileMimeTypeInfo::GroupInfo::itemInfo( const QString& key ) const
{
    ItemInfo* item = m_itemDict.find( key );

    // if we the item isn't found and variable keys are supported, we need to
    // return the default variable key iteminfo.
    if (!item && m_variableItemInfo)
    {
        return m_variableItemInfo;
    }
    return item;
}

KFileMimeTypeInfo::ItemInfo* KFileMimeTypeInfo::GroupInfo::addItemInfo(
                  const QString& key, const QString& translatedKey,
                  QVariant::Type type)
{
    kdDebug(7034) << key << "(" << translatedKey << ") -> " << QVariant::typeToName(type) << endl;

    ItemInfo* item = new ItemInfo(key, translatedKey, type);
    m_supportedKeys.append(key);
    m_itemDict.insert(key, item);
    return item;
}


void KFileMimeTypeInfo::GroupInfo::addVariableInfo( QVariant::Type type,
                                                   uint attr )
{
    // just make sure that it's not already there
    delete m_variableItemInfo;
    m_variableItemInfo = new ItemInfo(QString::null, QString::null, type);
    m_variableItemInfo->m_attr = attr;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////




// stream operators

/* serialization of a KFileMetaInfoItem:
   first a bool that says if the items is valid, and if yes,
   all the elements of the Data
*/
QDataStream& operator <<(QDataStream& s, const KFileMetaInfoItem& item )
{

     KFileMetaInfoItem::Data* d = item.d;

     // if the object is invalid, put only a char in the stream
     bool isValid = item.isValid();
     s << isValid;
     // ### what do about mimetypeInfo ?
     if (isValid)
         s << d->key
           << d->value
           << d->dirty
           << d->added
           << d->removed;

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

     // ### what do about mimetypeInfo ?
     bool dirty, added, removed;
     s >> item.d->key
       >> item.d->value
       >> dirty
       >> added
       >> removed;
     item.d->dirty = dirty;
     item.d->added = added;
     item.d->removed = removed;

    return s;
}


// serialization of a KFileMetaInfoGroup
// we serialize the name of the mimetype here instead of the mimetype info
// on the other side, we can simply use this to ask the provider for the info
QDataStream& operator <<(QDataStream& s, const KFileMetaInfoGroup& group )
{
    KFileMetaInfoGroup::Data* d = group.d;

    // if the object is invalid, put only a byte in the stream
    bool isValid = group.isValid();

    s << isValid;
    if (isValid)
    {
        s << d->name
          << d->items
          << d->mimeTypeInfo->mimeType();
    }
    return s;
};

QDataStream& operator >>(QDataStream& s, KFileMetaInfoGroup& group )
{
    QString mimeType;
    bool isValid;
    s >> isValid;

    // if it's invalid, there is not much to do
    if (!isValid)
    {
        group = KFileMetaInfoGroup();
        return s;
    }

    // we need a new object for our data
    group.deref();
    group.d = new KFileMetaInfoGroup::Data();
    group.ref();

    s >> group.d->name
      >> group.d->items
      >> mimeType;

    group.d->mimeTypeInfo = KFileMetaInfoProvider::self()->mimeTypeInfo(mimeType);

    // we need to set the item info for the items here
    QMapIterator<QString, KFileMetaInfoItem> it = group.d->items.begin();
    for ( ; it != group.d->items.end(); ++it)
    {
        (*it).d->mimeTypeInfo = group.d->mimeTypeInfo->groupInfo(group.d->name)
                                  ->itemInfo((*it).key());
    }
    
    return s;
}

// serialization of a KFileMetaInfo object
// we serialize the name of the mimetype here instead of the mimetype info
// on the other side, we can simply use this to ask the provider for the info
QDataStream& operator <<(QDataStream& s, const KFileMetaInfo& info )
{
    KFileMetaInfo::Data* d = info.d;

    // if the object is invalid, put only a byte that tells this
    bool isValid = info.isValid();

    s << isValid;
    if (isValid)
    {
        s << d->path
          << d->what
          << d->groups
          << d->mimeTypeInfo->mimeType();
    }
    return s;
};

QDataStream& operator >>(QDataStream& s, KFileMetaInfo& info )
{
    QString mimeType;
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

    s >> info.d->path
      >> info.d->what
      >> info.d->groups
      >> mimeType;
    info.d->mimeTypeInfo = KFileMetaInfoProvider::self()->mimeTypeInfo(mimeType);

    return s;
}




#include "kfilemetainfo.moc"
