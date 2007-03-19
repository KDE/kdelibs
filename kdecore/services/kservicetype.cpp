/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *                     David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kservicetype.h"
#include "ksycoca.h"
#include "kservice.h"
#include "kservicetypefactory.h"
#include "kservicefactory.h"
#include "kservicetypeprofile.h"
#include <assert.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <kconfiggroup.h>

template QDataStream& operator>> <QString, QVariant>(QDataStream&, QMap<QString, QVariant>&);
template QDataStream& operator<< <QString, QVariant>(QDataStream&, const QMap<QString, QVariant>&);

class KServiceTypePrivate
{
public:
    KServiceTypePrivate()
        : m_serviceOffersOffset( -1 ), m_parentTypeLoaded(false)
    {
    }

    void init( KDesktopFile *config, KServiceType* q );

    KServiceType::Ptr parentType;
    QString m_strName;
    QString m_strComment;
    int m_serviceOffersOffset;
    QMap<QString,QVariant::Type> m_mapPropDefs;
    bool m_bValid;
    bool m_bDerived;
    bool m_parentTypeLoaded;

};

KServiceType::KServiceType( const QString & _fullpath, const QString& _name,
                            const QString& _comment )
    : KSycocaEntry(_fullpath),
      d(new KServiceTypePrivate)
{
    d->m_strName = _name;
    d->m_strComment = _comment;
    d->m_bValid = !_name.isEmpty();
}

#if 0
KServiceType::KServiceType( const QString & _fullpath)
    : KSycocaEntry(_fullpath),
      d(new KServiceTypePrivate)
{
    KDesktopFile config( _fullpath );
    d->init(&config, this);
}
#endif

KServiceType::KServiceType( KDesktopFile *config )
    : KSycocaEntry(config->fileName()),
      d(new KServiceTypePrivate)
{
    d->init(config, this);
}

void
KServiceTypePrivate::init( KDesktopFile *config, KServiceType* q )
{
    KConfigGroup desktopGroup = config->desktopGroup();
    // Is it a mimetype ? ### KDE4: remove
    m_strName = desktopGroup.readEntry( "MimeType" );

    // Or is it a servicetype ?
    if ( m_strName.isEmpty() ) {
        m_strName = desktopGroup.readEntry( "X-KDE-ServiceType" );
    }

    m_strComment = desktopGroup.readEntry("Comment");
    q->setDeleted( desktopGroup.readEntry("Hidden", false) );

    // We store this as property to preserve BC, we can't change that
    // because KSycoca needs to remain BC between KDE 2.x and KDE 3.x
    QString sDerived = desktopGroup.readEntry( "X-KDE-Derived" );
    m_bDerived = !sDerived.isEmpty();
    if ( m_bDerived )
        q->m_mapProps.insert( "X-KDE-Derived", sDerived );

    const QStringList tmpList = config->groupList();
    QStringList::const_iterator gIt = tmpList.begin();

    for( ; gIt != tmpList.end(); ++gIt ) {
        if ( (*gIt).startsWith( "Property::" ) ) {
            KConfigGroup cg(config, *gIt );
            QVariant v = QVariant::nameToType( cg.readEntry( "Type" ).toLatin1().constData() );
            v = cg.readEntry( "Value", v );

            if ( v.isValid() )
                q->m_mapProps.insert( (*gIt).mid( 10 ), v );
        }
    }

    gIt = tmpList.begin();
    for( ; gIt != tmpList.end(); ++gIt ) {
        if( (*gIt).startsWith( "PropertyDef::" ) ) {
            KConfigGroup cg(config, *gIt);
            m_mapPropDefs.insert( (*gIt).mid( 13 ),
                                  QVariant::nameToType( cg.readEntry( "Type" ).toLatin1().constData() ) );
        }
    }

    m_bValid = !m_strName.isEmpty();
}

KServiceType::KServiceType( QDataStream& _str, int offset )
    : KSycocaEntry( _str, offset ), d(new KServiceTypePrivate)
{
    load( _str);
}

void
KServiceType::load( QDataStream& _str )
{
    qint8 b;
    QString dummy;
    _str >> d->m_strName >> dummy >> d->m_strComment >> m_mapProps >> d->m_mapPropDefs
         >> b >> d->m_serviceOffersOffset;
    d->m_bValid = b;
    d->m_bDerived = m_mapProps.contains("X-KDE-Derived");
}

void
KServiceType::save( QDataStream& _str )
{
  KSycocaEntry::save( _str );
  // !! This data structure should remain binary compatible at all times !!
  // You may add new fields at the end. Make sure to update the version
  // number in ksycoca.h
  _str << d->m_strName << QString() /*was icon*/ << d->m_strComment << m_mapProps << d->m_mapPropDefs
       << (qint8)d->m_bValid << d->m_serviceOffersOffset;
}

KServiceType::~KServiceType()
{
    delete d;
}

QString KServiceType::parentServiceType() const
{
    const QVariant v = property("X-KDE-Derived");
    return v.toString();
}

bool KServiceType::inherits( const QString& servTypeName ) const
{
    if ( name() == servTypeName )
        return true;
    QString st = parentServiceType();
    while ( !st.isEmpty() )
    {
        KServiceType::Ptr ptr = KServiceType::serviceType( st );
        if (!ptr) return false; //error
        if ( ptr->name() == servTypeName )
            return true;
        st = ptr->parentServiceType();
    }
    return false;
}

QVariant
KServiceType::property( const QString& _name ) const
{
    QVariant v;

    if ( _name == "Name" )
        v = QVariant( d->m_strName );
    else if ( _name == "Comment" )
        v = QVariant( d->m_strComment );
    else
        v = m_mapProps.value( _name );

    return v;
}

QStringList
KServiceType::propertyNames() const
{
    QStringList res = m_mapProps.keys();
    res.append( "Name" );
    res.append( "Comment" );
    return res;
}

QVariant::Type
KServiceType::propertyDef( const QString& _name ) const
{
    return static_cast<QVariant::Type>( d->m_mapPropDefs.value( _name, QVariant::Invalid ) );
}

QStringList
KServiceType::propertyDefNames() const
{
    return d->m_mapPropDefs.keys();
}

KServiceType::Ptr KServiceType::serviceType( const QString& _name )
{
    return KServiceTypeFactory::self()->findServiceTypeByName( _name );
}

KServiceType::List KServiceType::allServiceTypes()
{
    return KServiceTypeFactory::self()->allServiceTypes();
}

KServiceType::Ptr KServiceType::parentType()
{
    if (d->m_parentTypeLoaded)
        return d->parentType;

    d->m_parentTypeLoaded = true;

    const QString parentSt = parentServiceType();
    if (parentSt.isEmpty())
        return KServiceType::Ptr();

    d->parentType = KServiceTypeFactory::self()->findServiceTypeByName( parentSt );
    if (!d->parentType)
        kWarning(7009) << "'" << desktopEntryPath() << "' specifies undefined mimetype/servicetype '"<< parentSt << "'" << endl;
    return d->parentType;
}

void KServiceType::setServiceOffersOffset( int offset )
{
    Q_ASSERT( offset != -1 );
    d->m_serviceOffersOffset = offset;
}

int KServiceType::serviceOffersOffset() const
{
    return d->m_serviceOffersOffset;
}

QString KServiceType::comment() const
{
    return d->m_strComment;
}

QString KServiceType::name() const
{
    return d->m_strName;
}

// ## KDE4: remove?
QString KServiceType::desktopEntryPath() const
{
    return entryPath();
}

bool KServiceType::isDerived() const
{
    return d->m_bDerived;
}

bool KServiceType::isValid() const
{
    return d->m_bValid;
}

const QMap<QString,QVariant::Type>& KServiceType::propertyDefs() const
{
     return d->m_mapPropDefs;
}

void KServiceType::virtual_hook( int id, void* data )
{ KSycocaEntry::virtual_hook( id, data ); }
