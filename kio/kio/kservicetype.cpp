/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *                     David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "kservice.h"
#include "ksycoca.h"
#include "kservicetype.h"
#include "kservicetypefactory.h"
#include "kservicefactory.h"
#include "kuserprofile.h"
#include <assert.h>
#include <kdebug.h>
#include <kdesktopfile.h>

template QDataStream& operator>> <QString, QVariant>(QDataStream&, QMap<QString, QVariant>&);
template QDataStream& operator<< <QString, QVariant>(QDataStream&, const QMap<QString, QVariant>&);
template QDataStream& operator>> <QString, QVariant::Type>(QDataStream&, QMap<QString, QVariant::Type>&);
template QDataStream& operator<< <QString, QVariant::Type>(QDataStream&, const QMap<QString, QVariant::Type>&);

KServiceType::KServiceType( const QString & _fullpath)
 : KSycocaEntry(_fullpath)
{
  KDesktopFile config( _fullpath );

  init(&config);
}

KServiceType::KServiceType( KDesktopFile *config )
 : KSycocaEntry(config->filename())
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
  m_bDeleted = config->readBoolEntry( "Hidden", false );
  m_strIcon = config->readIcon();

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
    if ( (*gIt).find( "Property::" ) == 0 )
    {
      config->setGroup( *gIt );
      QVariant v = config->readPropertyEntry( "Value",
                   QVariant::nameToType( config->readEntry( "Type" ).ascii() ) );
      if ( v.isValid() )
          m_mapProps.insert( (*gIt).mid( 10 ), v );
    }
  }

  gIt = tmpList.begin();
  for( ; gIt != tmpList.end(); ++gIt )
  {
    if( (*gIt).find( "PropertyDef::" ) == 0 )
    {
      config->setGroup( *gIt );
      m_mapPropDefs.insert( (*gIt).mid( 13 ),
			    QVariant::nameToType( config->readEntry( "Type" ).ascii() ) );
    }
  }

  m_bValid = !m_strName.isEmpty();
}

KServiceType::KServiceType( const QString & _fullpath, const QString& _type,
                            const QString& _icon, const QString& _comment )
 : KSycocaEntry(_fullpath)
{
  m_strName = _type;
  m_strIcon = _icon;
  m_strComment = _comment;
  m_bValid = !m_strName.isEmpty();
}

KServiceType::KServiceType( QDataStream& _str, int offset )
 : KSycocaEntry( _str, offset )
{
  load( _str);
}

void
KServiceType::load( QDataStream& _str )
{
  Q_INT8 b;
  _str >> m_strName >> m_strIcon >> m_strComment >> m_mapProps >> m_mapPropDefs
       >> b;
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
  _str << m_strName << m_strIcon << m_strComment << m_mapProps << m_mapPropDefs
       << (Q_INT8)m_bValid;
}

KServiceType::~KServiceType()
{
}

QString KServiceType::parentServiceType() const
{
  QVariant v = property("X-KDE-Derived");
  return v.toString();
}

QVariant
KServiceType::property( const QString& _name ) const
{
  QVariant v;

  if ( _name == "Name" )
    v = QVariant( m_strName );
  else if ( _name == "Icon" )
    v = QVariant( m_strIcon );
  else if ( _name == "Comment" )
    v = QVariant( m_strComment );
  else {
    QMap<QString,QVariant>::ConstIterator it = m_mapProps.find( _name );
    if ( it != m_mapProps.end() )
      v = it.data();
  }

  return v;
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
    return QVariant::Invalid;
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
  KService::List lst;

  // Services associated directly with this servicetype (the normal case)
  KServiceType * serv = KServiceTypeFactory::self()->findServiceTypeByName( _servicetype );
  if ( serv )
    lst += KServiceFactory::self()->offers( serv->offset() );
  else
    kdWarning(7009) << "KServiceType::offers : servicetype " << _servicetype << " not found" << endl;
  bool isAMimeType = serv ? serv->isType( KST_KMimeType ) : false;
  delete serv;

  //QValueListIterator<KService::Ptr> it = lst.begin();
  //for( ; it != lst.end(); ++it )
  //    kdDebug() << (*it).data() << " " << (*it)->name() << endl;

  // Support for all/* is deactivated by KServiceTypeProfile::configurationMode()
  // (and makes no sense when querying for an "all" servicetype itself
  // nor for non-mimetypes service types)
  if ( !KServiceTypeProfile::configurationMode()
       && isAMimeType
       && _servicetype.left(4) != "all/" )
  {
    // Support for services associated with "all"
    KServiceType * servAll = KServiceTypeFactory::self()->findServiceTypeByName( "all/all" );
    if ( servAll )
    {
        KService::List newOffers = KServiceFactory::self()->offers( servAll->offset() );
        // Look if we already have those services from the initial query, to avoid duplicates
        QValueListIterator<KService::Ptr> it = newOffers.begin();
        for( ; it != newOffers.end(); ++it )
        {
            bool found = false;
            QValueListIterator<KService::Ptr> it2 = lst.begin();
            for( ; it2 != lst.end() && !found; ++it2 )
                found = (*it)->desktopEntryPath() == (*it2)->desktopEntryPath();
            if ( !found )
            {
                (*it)->setInitialPreference( 0 ); // all/* associations are less prioritary
                lst += *it;
            }
        }
        //kdDebug(7009) << "all/all found, got " << newOffers.count() << " more offers" << endl;
    }
    else
      kdWarning(7009) << "KServiceType::offers : servicetype all/all not found" << endl;
    delete servAll;

    // Support for services associated with "allfiles"
    if ( _servicetype != "inode/directory" && _servicetype != "inode/directory-locked" )
    {
      KServiceType * servAllFiles = KServiceTypeFactory::self()->findServiceTypeByName( "all/allfiles" );
      if ( servAllFiles )
      {
        KService::List newOffers = KServiceFactory::self()->offers( servAllFiles->offset() );
        // Look if we already have those services from the initial query, to avoid duplicates
        QValueListIterator<KService::Ptr> it = newOffers.begin();
        for( ; it != newOffers.end(); ++it )
        {
            bool found = false;
            QValueListIterator<KService::Ptr> it2 = lst.begin();
            for( ; it2 != lst.end() && !found; ++it2 )
                found = (*it)->desktopEntryPath() == (*it2)->desktopEntryPath();
            if ( !found )
            {
                (*it)->setInitialPreference( 0 ); // all/* associations are less prioritary
                lst += *it;
            }
        }
        //kdDebug(7009) << "all/allfiles found, got " << newOffers.count() << " more offers" << endl;
      }
      else
        kdWarning(7009) << "KServiceType::offers : servicetype all/allfiles not found" << endl;
      delete servAllFiles;
    }
  }

  return lst;
}

KServiceType::List KServiceType::allServiceTypes()
{
  return KServiceTypeFactory::self()->allServiceTypes();
}

void KServiceType::virtual_hook( int id, void* data )
{ KSycocaEntry::virtual_hook( id, data ); }
