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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "kservice.h"
#include "ksycoca.h"
#include "kservicetype.h"
#include "kservicetypefactory.h"
#include "kservicefactory.h"

#include <assert.h>
#include <kdebug.h>
#include <qsmartptr.h>

KServiceType::KServiceType( const QString & _fullpath, const QString & _resource )
{
  m_strRelativeFilePath = KSycoca::determineRelativePath( _fullpath, _resource );
  KSimpleConfig config( _fullpath, true );
  config.setDesktopGroup();

  // Is it a mimetype ?
  m_strName = config.readEntry( "MimeType" );

  // Or is it a servicetype ?
  if ( m_strName.isEmpty() ) 
  {
    m_strName = config.readEntry( "X-KDE-ServiceType" );
  }

  m_strComment = config.readEntry( "Comment" );
  m_strIcon = config.readEntry( "Icon" );

  QStringList tmpList = config.groupList();
  QStringList::Iterator gIt = tmpList.begin();

  for( ; gIt != tmpList.end(); ++gIt )
  {
    if ( (*gIt).find( "Property::" ) == 0 )
    {
      config.setGroup( *gIt );
      m_mapProps.insert( (*gIt).mid( 10 ),
			 config.readPropertyEntry( "Value",
						 QVariant::nameToType( config.readEntry( "Type" ) ) ) );
    }
  }

  gIt = tmpList.begin();
  for( ; gIt != tmpList.end(); ++gIt )
  {
    if( (*gIt).find( "PropertyDef::" ) == 0 )
    {
      config.setGroup( *gIt );
      m_mapPropDefs.insert( (*gIt).mid( 13 ),
			    QVariant::nameToType( config.readEntry( "Type" ) ) );
    }
  }

  m_bValid = !m_strName.isEmpty();
}

KServiceType::KServiceType( const QString & _fullpath, const QString& _type, 
                            const QString& _icon, const QString& _comment, const QString & _resource )
{
  m_strRelativeFilePath = KSycoca::determineRelativePath( _fullpath, _resource );
  m_strName = _type;
  m_strIcon = _icon;
  m_strComment = _comment;
  m_bValid = !m_strName.isEmpty();
}

KServiceType::KServiceType( QDataStream& _str, int offset ) : KSycocaEntry( _str, offset )
{
  load( _str);
}

void
KServiceType::load( QDataStream& _str )
{
  Q_INT8 b;
  _str >> m_strName >> m_strIcon >> m_strComment >> m_mapProps >> m_mapPropDefs 
       >> b >> m_strRelativeFilePath;
  m_bValid = b;
}

void 
KServiceType::save( QDataStream& _str )
{
  KSycocaEntry::save( _str );
  // Warning adding/removing fields here involves a binary incompatible change - update version 
  // number in ksycoca.h
  _str << m_strName << m_strIcon << m_strComment << m_mapProps << m_mapPropDefs 
       << (Q_INT8)m_bValid << m_strRelativeFilePath;
}

KServiceType::~KServiceType()
{
}

KServiceType::PropertyPtr 
KServiceType::property( const QString& _name ) const
{
  QVariant* p = 0;

  if ( _name == "Name" )
    p = new QVariant( m_strName );
  if ( _name == "Icon" )
    p = new QVariant( m_strIcon );
  if ( _name == "Comment" )
    p = new QVariant( m_strComment );

  if ( p )
    return KServiceType::PropertyPtr( p );

  QMap<QString,QVariant>::ConstIterator it = m_mapProps.find( _name );
  if ( it == m_mapProps.end() )
    return (QVariant*)0;

  p = (QVariant*)(&(it.data()));

  p->ref();
  return KServiceType::PropertyPtr( p );
}

QStringList 
KServiceType::propertyNames() const
{
  QStringList res;

  QMap<QString,QVariant>::ConstIterator it = m_mapProps.begin();
  for( ; it != m_mapProps.end(); ++it )
    res.append( it.key() );

  res.append( "Name" );
  res.append( "Comment" );
  res.append( "Icon" );

  return res;
}

QVariant::Type 
KServiceType::propertyDef( const QString& _name ) const
{
  QMap<QString,QVariant::Type>::ConstIterator it = m_mapPropDefs.find( _name );
  if ( it == m_mapPropDefs.end() )
    return QVariant::Empty;
  return it.data();
}

QStringList 
KServiceType::propertyDefNames() const
{
  QStringList l;

  QMap<QString,QVariant::Type>::ConstIterator it = m_mapPropDefs.begin();
  for( ; it != m_mapPropDefs.end(); ++it )
    l.append( it.key() );

  return l;
}

KServiceType::Ptr KServiceType::serviceType( const QString& _name )
{
  KServiceType * p = KServiceTypeFactory::self()->findServiceTypeByName( _name );
  return KServiceType::Ptr( p );
}

KService::List KServiceType::offers( const QString& _servicetype )
{
  KServiceType * serv = KServiceTypeFactory::self()->findServiceTypeByName( _servicetype );
  if ( serv )
    return KServiceFactory::self()->offers( serv->offset() );
  else
  {
    kdebug(KDEBUG_WARN, 7009, QString("KServiceType::offers : servicetype %1 not found").arg( _servicetype ));
    KService::List l;
    return l;
  }
}

KServiceType::List KServiceType::allServiceTypes()
{
  return KServiceTypeFactory::self()->allServiceTypes();
}

