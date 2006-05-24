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

#include "ksycoca.h"
#include "kservicetype.h"
#include "kservicetypefactory.h"
#include "kservicefactory.h"
#include "kservicetypeprofile.h"
#include <assert.h>
#include <kdebug.h>
#include <kdesktopfile.h>

template QDataStream& operator>> <QString, QVariant>(QDataStream&, QMap<QString, QVariant>&);
template QDataStream& operator<< <QString, QVariant>(QDataStream&, const QMap<QString, QVariant>&);

class KServiceType::KServiceTypePrivate
{
public:
  KServiceTypePrivate() { }

  KServiceType::Ptr parentType;
};

KServiceType::KServiceType( const QString & _fullpath, const QString& _type,
                            const QString& _comment )
  : KSycocaEntry(_fullpath),
    m_serviceOffersOffset( -1 ), m_parentTypeLoaded(false), d(0)
{
  m_strName = _type;
  m_strComment = _comment;
  m_bValid = !m_strName.isEmpty();
}

KServiceType::KServiceType( const QString & _fullpath)
  : KSycocaEntry(_fullpath),
    m_serviceOffersOffset( -1 ), m_parentTypeLoaded(false), d(0)
{
  KDesktopFile config( _fullpath );

  init(&config);
}

KServiceType::KServiceType( KDesktopFile *config )
  : KSycocaEntry(config->fileName()),
    m_serviceOffersOffset( -1 ), m_parentTypeLoaded(false), d(0)
{
  init(config);
}

void
KServiceType::init( KDesktopFile *config)
{
  // Is it a mimetype ?
  m_strName = config->readEntry( "MimeType" );

  // Or is it a servicetype ?
  if ( m_strName.isEmpty() )
  {
    m_strName = config->readEntry( "X-KDE-ServiceType" );
  }

  m_strComment = config->readComment();
  m_bDeleted = config->readEntry("Hidden", false);

  // We store this as property to preserve BC, we can't change that
  // because KSycoca needs to remain BC between KDE 2.x and KDE 3.x
  QString sDerived = config->readEntry( "X-KDE-Derived" );
  m_bDerived = !sDerived.isEmpty();
  if ( m_bDerived )
    m_mapProps.insert( "X-KDE-Derived", sDerived );

  QStringList tmpList = config->groupList();
  QStringList::Iterator gIt = tmpList.begin();

  for( ; gIt != tmpList.end(); ++gIt )
  {
    if ( (*gIt).startsWith( "Property::" ) )
    {
      config->setGroup( *gIt );
      QVariant v = QVariant::nameToType( config->readEntry( "Type" ).toLatin1().constData() );
      v = config->readEntry( "Value", v );

      if ( v.isValid() )
          m_mapProps.insert( (*gIt).mid( 10 ), v );
    }
  }

  gIt = tmpList.begin();
  for( ; gIt != tmpList.end(); ++gIt )
  {
    if( (*gIt).startsWith( "PropertyDef::" ) )
    {
      config->setGroup( *gIt );
      m_mapPropDefs.insert( (*gIt).mid( 13 ),
			    QVariant::nameToType( config->readEntry( "Type" ).toLatin1().constData() ) );
    }
  }

  m_bValid = !m_strName.isEmpty();
}

KServiceType::KServiceType( QDataStream& _str, int offset )
 : KSycocaEntry( _str, offset ), d(0)
{
  load( _str);
}

void
KServiceType::load( QDataStream& _str )
{
  qint8 b;
  QString dummy;
  _str >> m_strName >> dummy >> m_strComment >> m_mapProps >> m_mapPropDefs
       >> b >> m_serviceOffersOffset;
  m_bValid = b;
  m_bDerived = m_mapProps.contains("X-KDE-Derived");
}

void
KServiceType::save( QDataStream& _str )
{
  KSycocaEntry::save( _str );
  // !! This data structure should remain binary compatible at all times !!
  // You may add new fields at the end. Make sure to update the version
  // number in ksycoca.h
  _str << m_strName << QString() /*was icon*/ << m_strComment << m_mapProps << m_mapPropDefs
       << (qint8)m_bValid << m_serviceOffersOffset;
}

KServiceType::~KServiceType()
{
  delete d;
}

QString KServiceType::parentServiceType() const
{
  QVariant v = property("X-KDE-Derived");
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
    v = QVariant( m_strName );
  else if ( _name == "Comment" )
    v = QVariant( m_strComment );
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
  return static_cast<QVariant::Type>( m_mapPropDefs.value( _name, QVariant::Invalid ) );
}

QStringList
KServiceType::propertyDefNames() const
{
  return m_mapPropDefs.keys();
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
  if (m_parentTypeLoaded)
    return d ? d->parentType : KServiceType::Ptr();

  m_parentTypeLoaded = true;

  QString parentSt = parentServiceType();
  if (parentSt.isEmpty())
    return KServiceType::Ptr();

  if (!d)
    d = new KServiceTypePrivate;
  d->parentType = KServiceTypeFactory::self()->findServiceTypeByName( parentSt );
  if (!d->parentType)
    kWarning(7009) << "'" << desktopEntryPath() << "' specifies undefined mimetype/servicetype '"<< parentSt << "'" << endl;
  return d->parentType;
}

void KServiceType::setServiceOffersOffset( int offset )
{
  Q_ASSERT( offset != -1 );
  m_serviceOffersOffset = offset;
}

int KServiceType::serviceOffersOffset() const
{
  return m_serviceOffersOffset;
}

void KServiceType::virtual_hook( int id, void* data )
{ KSycocaEntry::virtual_hook( id, data ); }
