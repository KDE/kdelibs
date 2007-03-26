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
#include "kservicetype_p.h"
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

KServiceType::KServiceType( KServiceTypePrivate &dd, const QString & _fullpath, const QString& _name,
                            const QString& _comment )
    : KSycocaEntry(_fullpath),
      d_ptr(&dd)
{
    Q_D(KServiceType);
    d->m_strName = _name;
    d->m_strComment = _comment;
    d->m_bValid = !_name.isEmpty();
}

#if 0
KServiceType::KServiceType( const QString & _fullpath)
    : KSycocaEntry(_fullpath),
      d_ptr(new KServiceTypePrivate(this))
{
    Q_D(KServiceType);
    KDesktopFile config( _fullpath );
    d->init(&config);
}
#endif

KServiceType::KServiceType( KDesktopFile *config )
    : KSycocaEntry(config->fileName()),
      d_ptr(new KServiceTypePrivate(this))
{
    Q_D(KServiceType);
    d->init(config);
}

void
KServiceTypePrivate::init( KDesktopFile *config )
{
    Q_Q(KServiceType);

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
        m_mapProps.insert( "X-KDE-Derived", sDerived );

    const QStringList tmpList = config->groupList();
    QStringList::const_iterator gIt = tmpList.begin();

    for( ; gIt != tmpList.end(); ++gIt ) {
        if ( (*gIt).startsWith( "Property::" ) ) {
            KConfigGroup cg(config, *gIt );
            QVariant v = QVariant::nameToType( cg.readEntry( "Type" ).toLatin1().constData() );
            v = cg.readEntry( "Value", v );

            if ( v.isValid() )
                m_mapProps.insert( (*gIt).mid( 10 ), v );
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
    : KSycocaEntry( _str, offset ), d_ptr(new KServiceTypePrivate(this))
{
    load( _str);
}

KServiceType::KServiceType( KServiceTypePrivate &dd, QDataStream& _str, int offset )
    : KSycocaEntry( _str, offset ), d_ptr(&dd)
{
    load( _str);
}

void
KServiceType::load( QDataStream& _str )
{
    Q_D(KServiceType);
    qint8 b;
    QString dummy;
    _str >> d->m_strName >> dummy >> d->m_strComment >> d->m_mapProps >> d->m_mapPropDefs
         >> b >> d->m_serviceOffersOffset;
    d->m_bValid = b;
    d->m_bDerived = d->m_mapProps.contains("X-KDE-Derived");
}

void
KServiceType::save( QDataStream& _str )
{
  Q_D(KServiceType);
  KSycocaEntry::save( _str );
  // !! This data structure should remain binary compatible at all times !!
  // You may add new fields at the end. Make sure to update the version
  // number in ksycoca.h
  _str << d->m_strName << QString() /*was icon*/ << d->m_strComment << d->m_mapProps << d->m_mapPropDefs
       << (qint8)d->m_bValid << d->m_serviceOffersOffset;
}

KServiceType::~KServiceType()
{
    delete d_ptr;
}

QString KServiceType::parentServiceType() const
{
    Q_D(const KServiceType);
    const QVariant v = property("X-KDE-Derived");
    return v.toString();
}

bool KServiceType::inherits( const QString& servTypeName ) const
{
    Q_D(const KServiceType);
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
    Q_D(const KServiceType);
    QVariant v;

    if ( _name == "Name" )
        v = QVariant( d->m_strName );
    else if ( _name == "Comment" )
        v = QVariant( d->m_strComment );
    else
        v = d->m_mapProps.value( _name );

    return v;
}

QStringList
KServiceType::propertyNames() const
{
    Q_D(const KServiceType);
    QStringList res = d->m_mapProps.keys();
    res.append( "Name" );
    res.append( "Comment" );
    return res;
}

QVariant::Type
KServiceType::propertyDef( const QString& _name ) const
{
    Q_D(const KServiceType);
    return static_cast<QVariant::Type>( d->m_mapPropDefs.value( _name, QVariant::Invalid ) );
}

QStringList
KServiceType::propertyDefNames() const
{
    Q_D(const KServiceType);
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
    Q_D(KServiceType);
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
    Q_D(KServiceType);
    Q_ASSERT( offset != -1 );
    d->m_serviceOffersOffset = offset;
}

int KServiceType::serviceOffersOffset() const
{
    Q_D(const KServiceType);
    return d->m_serviceOffersOffset;
}

QString KServiceType::comment() const
{
    Q_D(const KServiceType);
    return d->m_strComment;
}

QString KServiceType::name() const
{
    Q_D(const KServiceType);
    return d->m_strName;
}

// ## KDE4: remove?
QString KServiceType::desktopEntryPath() const
{
    return entryPath();
}

bool KServiceType::isDerived() const
{
    Q_D(const KServiceType);
    return d->m_bDerived;
}

bool KServiceType::isValid() const
{
    Q_D(const KServiceType);
    return d->m_bValid;
}

const QMap<QString,QVariant::Type>& KServiceType::propertyDefs() const
{
    Q_D(const KServiceType);
    return d->m_mapPropDefs;
}

void KServiceType::virtual_hook( int id, void* data )
{ KSycocaEntry::virtual_hook( id, data ); }
