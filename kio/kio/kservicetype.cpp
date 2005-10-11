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

#include "kservice.h"
#include "ksycoca.h"
#include "kservicetype.h"
#include "kservicetypefactory.h"
#include "kservicefactory.h"
#include "kuserprofile.h"
#include <assert.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <q3dict.h>

template QDataStream& operator>> <QString, QVariant>(QDataStream&, QMap<QString, QVariant>&);
template QDataStream& operator<< <QString, QVariant>(QDataStream&, const QMap<QString, QVariant>&);

class KServiceType::KServiceTypePrivate
{
public:
  KServiceTypePrivate() : parentTypeLoaded(false) { }

  KServiceType::Ptr parentType;
  KService::List services;
  bool parentTypeLoaded;
};

KServiceType::KServiceType( const QString & _fullpath)
 : KSycocaEntry(_fullpath), d(0)
{
  KDesktopFile config( _fullpath );

  init(&config);
}

KServiceType::KServiceType( KDesktopFile *config )
 : KSycocaEntry(config->fileName()), d(0)
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
 : KSycocaEntry(_fullpath), d(0)
{
  m_strName = _type;
  m_strIcon = _icon;
  m_strComment = _comment;
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
       << (qint8)m_bValid;
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

static void addUnique(KService::List &lst, Q3Dict<KService> &dict, const KService::List &newLst, bool lowPrio)
{
  Q3ValueListConstIterator<KService::Ptr> it = newLst.begin();
  for( ; it != newLst.end(); ++it )
  {
     KService *service = static_cast<KService*>(*it);
     if (dict.find(service->desktopEntryPath()))
        continue;
     dict.insert(service->desktopEntryPath(), service);
     lst.append(service);
     if (lowPrio)
        service->setInitialPreference( 0 );
  }
}

KService::List KServiceType::offers( const QString& _servicetype )
{
  Q3Dict<KService> dict(53);
  KService::List lst;

  // Services associated directly with this servicetype (the normal case)
  KServiceType::Ptr serv = KServiceTypeFactory::self()->findServiceTypeByName( _servicetype );
  if ( serv )
    addUnique(lst, dict, KServiceFactory::self()->offers( serv->offset() ), false);
  else
    kdWarning(7009) << "KServiceType::offers : servicetype " << _servicetype << " not found" << endl;

  // Find services associated with any mimetype parents. e.g. text/x-java -> text/plain    
  KMimeType::Ptr mime = dynamic_cast<KMimeType*>(static_cast<KServiceType *>(serv));
  bool isAMimeType = (mime != 0);
  if (mime)
  {
     while(true)
     {
        QString parent = mime->parentMimeType();
        if (parent.isEmpty())
           break;
        mime = dynamic_cast<KMimeType *>(KServiceTypeFactory::self()->findServiceTypeByName( parent ));
        if (!mime)
           break;
        
        addUnique(lst, dict, KServiceFactory::self()->offers( mime->offset() ), false);
     }
  }
  serv = mime = 0;

  //QValueListIterator<KService::Ptr> it = lst.begin();
  //for( ; it != lst.end(); ++it )
  //    kdDebug() << (*it).data() << " " << (*it)->name() << endl;

  // Support for all/* is deactivated by KServiceTypeProfile::configurationMode()
  // (and makes no sense when querying for an "all" servicetype itself
  // nor for non-mimetypes service types)
  if ( !KServiceTypeProfile::configurationMode()
       && isAMimeType
       && !_servicetype.startsWith( QLatin1String( "all/" ) ) )
  {
    // Support for services associated with "all"
    KServiceType * servAll = KServiceTypeFactory::self()->findServiceTypeByName( "all/all" );
    if ( servAll )
    {
        addUnique(lst, dict, KServiceFactory::self()->offers( servAll->offset() ), true);
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
        addUnique(lst, dict, KServiceFactory::self()->offers( servAllFiles->offset() ), true);
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

KServiceType::Ptr KServiceType::parentType()
{
  if (d && d->parentTypeLoaded)
     return d->parentType;
  
  if (!d)
     d = new KServiceTypePrivate;
     
  QString parentSt = parentServiceType();
  if (!parentSt.isEmpty())
  {
    d->parentType = KServiceTypeFactory::self()->findServiceTypeByName( parentSt );
    if (!d->parentType)
      kdWarning(7009) << "'" << desktopEntryPath() << "' specifies undefined mimetype/servicetype '"<< parentSt << "'" << endl;
  }
  
  d->parentTypeLoaded = true;

  return d->parentType;
}

void KServiceType::addService(KService::Ptr service)
{
  if (!d)
     d = new KServiceTypePrivate;
  
  if (d->services.count() && d->services.last() == service)
     return;
     
  d->services.append(service);
}

KService::List KServiceType::services() const
{
  if (d)
     return d->services;

  return KService::List();
}

void KServiceType::virtual_hook( int id, void* data )
{ KSycocaEntry::virtual_hook( id, data ); }
