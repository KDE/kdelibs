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

#include "kservicetype.h"
#include "kservicetypefactory.h"

#include <assert.h>
#include <qsmartptr.h>

KServiceType::KServiceType( KSimpleConfig& _cfg )
{
  _cfg.setDesktopGroup();

  // Is it a mimetype ?
  m_strName = _cfg.readEntry( "MimeType" );

  // Or is it a servicetype ?
  if ( m_strName.isEmpty() ) 
  {
    m_strName = _cfg.readEntry( "X-KDE-ServiceType" );
  }

  m_strComment = _cfg.readEntry( "Comment" );
  m_strIcon = _cfg.readEntry( "Icon" );

  QStringList tmpList = _cfg.groupList();
  QStringList::Iterator gIt = tmpList.begin();

  for( ; gIt != tmpList.end(); ++gIt )
  {
    if ( (*gIt).find( "Property::" ) == 0 )
    {
      _cfg.setGroup( *gIt );
      m_mapProps.insert( (*gIt).mid( 10 ),
			 _cfg.readPropertyEntry( "Value",
						 QVariant::nameToType( _cfg.readEntry( "Type" ) ) ) );
    }
  }

  gIt = tmpList.begin();
  for( ; gIt != tmpList.end(); ++gIt )
  {
    if( (*gIt).find( "PropertyDef::" ) == 0 )
    {
      _cfg.setGroup( *gIt );
      m_mapPropDefs.insert( (*gIt).mid( 13 ),
			    QVariant::nameToType( _cfg.readEntry( "Type" ) ) );
    }
  }

  m_bValid = !m_strName.isEmpty();
}

KServiceType::KServiceType( const QString& _type, const QString& _icon, const QString& _comment )
{
  m_strName = _type;
  m_strIcon = _icon;
  m_strComment = _comment;
  m_bValid = !m_strName.isEmpty();
}

KServiceType::KServiceType( QDataStream& _str ) : KSycocaEntry( _str )
{
  load( _str);
}

void
KServiceType::load( QDataStream& _str )
{
  Q_INT8 b;
  _str >> m_strName >> m_strIcon >> m_strComment >> m_mapProps >> m_mapPropDefs >> b;
  m_bValid = b;
}

void 
KServiceType::save( QDataStream& _str )
{
  KSycocaEntry::save( _str );
  _str << m_strName << m_strIcon << m_strComment << m_mapProps << m_mapPropDefs << (Q_INT8)m_bValid;
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

KServiceType* 
KServiceType::serviceType( const QString& _name )
{
  return KServiceTypeFactory::findServiceTypeByName( _name );
}

/*
QDataStream& operator>>( QDataStream& _str, KServiceType& s )
{
  s.load( _str );
  return _str;
}

QDataStream& operator<<( QDataStream& _str, KServiceType& s )
{
  s.save( _str );
  return _str;
}
*/
