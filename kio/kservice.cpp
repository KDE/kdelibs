/*  This file is part of the KDE libraries
 *  Copyright (C) 1999, 2000 Waldo Bastian <bastian@kde.org>
 *  Copyright (C) 1999       David Faure   <faure@kde.org>
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

// $Id$


#include <sys/types.h>
#include <sys/stat.h>

#include <stddef.h>
#include <unistd.h>

#include <qstring.h>
#include <qtl.h>

#include <ksimpleconfig.h>
#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <dcopclient.h>

#include "kservicefactory.h"
#include "kservicetypefactory.h"
#include "kservice.h"
#include "kservicetype.h"
#include "kuserprofile.h"
#include "ksycoca.h"

KService::KService( const QString & _fullpath )
{
  KDesktopFile config( _fullpath );

  init(&config);
}

KService::KService( KDesktopFile *config )
{
  init(config);
}

void
KService::init( KDesktopFile *config )
{
  m_bValid = true;

  m_strDesktopEntryPath = config->filename();

  config->setDesktopGroup();
  m_strType = config->readEntry( "Type" );
  if ( m_strType.isEmpty() )
  {
    kDebugWarning( 7012, QString("The desktop entry file %1 has no Type=... entry. "
            "It should be \"Application\", or \"Service\"")
            .arg( m_strDesktopEntryPath ) );
    m_bValid = false;
    return;
  }
  if ( m_strType != "Application" && m_strType != "Service" )
  {
    kDebugWarning( 7012, QString("The desktop entry file %1 has Type=%2 "
            "instead of \"Application\" or \"Service\"")
            .arg( m_strDesktopEntryPath ).arg(m_strType) );
    m_bValid = false;
    return;
  }
  QString resource = config->resource();

  if ( (m_strType == "Application") && 
       (!resource.isEmpty()) &&
       (resource != "apps"))
  {
    kDebugWarning( 7012, QString("The desktop entry file %1 has Type=%2 "
            "but is located under \"%3\" instead of \"apps\"")
            .arg( m_strDesktopEntryPath ).arg(m_strType).arg( resource ));
    m_bValid = false;
    return;
  }

  if ( (m_strType == "Service") && 
       (!resource.isEmpty()) &&
       (resource != "services"))
  {
    kDebugWarning( 7012, QString("The desktop entry file %1 has Type=%2 "
            "but is located under \"%3\" instead of \"services\"")
            .arg( m_strDesktopEntryPath ).arg(m_strType).arg( resource ));
    m_bValid = false;
    return;
  }

  m_strExec = config->readEntry( "Exec" );
  m_strName = config->readEntry( "Name" );
  if ( m_strName.isEmpty() )
  {
    kDebugWarning( 7012, QString("The desktop entry file %1 has no Name")
            .arg( m_strDesktopEntryPath ) );
    m_bValid = false;
    return;
  }

  m_strIcon = config->readEntry( "Icon", "unknown.png" );
  m_strTerminalOptions = config->readEntry( "TerminalOptions" );
  m_strPath = config->readEntry( "Path" );
  m_strComment = config->readEntry( "Comment" );
  m_bDeleted = config->readBoolEntry( "Hidden", false );
  m_strLibrary = config->readEntry( "X-KDE-Library" );
  m_libraryMajor = config->readNumEntry( "X-KDE-LibraryMajor", 0 );
  m_libraryMinor = config->readNumEntry( "X-KDE-LibraryMinor", 0 );
  m_lstLibraryDeps = config->readListEntry( "X-KDE-LibraryDependencies" );
  m_lstServiceTypes = config->readListEntry( "ServiceTypes" );
  // For compatibility with KDE 1.x
  m_lstServiceTypes += config->readListEntry( "MimeType", ';' );

  QString dcopServiceType = config->readEntry("X-DCOP-ServiceType").lower();
  if (dcopServiceType == "unique")
     m_DCOPServiceType = DCOP_Unique;
  else if (dcopServiceType == "multi")
     m_DCOPServiceType = DCOP_Multi;
  else
     m_DCOPServiceType = DCOP_None;

  QString name = m_strDesktopEntryPath;
  int pos = name.findRev('/');
  if (pos != -1)
     name = name.mid(pos+1);
  pos = name.find('.');
  if (pos != -1)
     name = name.left(pos);

  m_strDesktopEntryName = name;
  
  if ( m_strType == "Application" )
    // Specify AllowDefault = false to explicitely forbid it.
    // Most service files don't have that field, so true is the default
    m_bAllowAsDefault = config->readBoolEntry( "AllowDefault", true );
  else
    // Doesn't exist for generic services, since KRun has to be able
    // to run the default service. It can't run a lib...
    m_bAllowAsDefault = false;

  // Load all additional properties
  QStringList::Iterator it = m_lstServiceTypes.begin();
  for( ; it != m_lstServiceTypes.end(); ++it )
  {
    KServiceType * s = KServiceTypeFactory::self()->findServiceTypeByName( *it );
    if ( s )
    {
      const QMap<QString,QVariant::Type>& pd = s->propertyDefs();
      QMap<QString,QVariant::Type>::ConstIterator pit = pd.begin();
      for( ; pit != pd.end(); ++pit )
      {
	m_mapProps.insert( pit.key(), config->readPropertyEntry( pit.key(), pit.data() ) );
      }
    }
  }

}

KService::KService( QDataStream& _str, int offset ) : KSycocaEntry( _str, offset )
{
  load( _str );
}

KService::~KService()
{
  //debug("KService::~KService()");
}

QPixmap KService::pixmap( KIconLoader::Size _size, QString * _path ) const
{
  return KGlobal::iconLoader()->loadIcon( m_strIcon, _size, _path );
}

void KService::load( QDataStream& s )
{
  Q_INT8 b;
  Q_INT8 dst;

  s >> m_strType >> m_strName >> m_strExec >> m_strIcon 
    >> m_strTerminalOptions
    >> m_strPath >> m_strComment >> m_lstServiceTypes >> b >> m_mapProps
    >> m_strLibrary >> m_libraryMajor >> m_libraryMinor 
    >> dst
    >> m_strDesktopEntryPath >> m_strDesktopEntryName;

  m_bAllowAsDefault = b;
  m_DCOPServiceType = (DCOPServiceType_t) dst;

  m_bValid = true;
}

void KService::save( QDataStream& s )
{
  KSycocaEntry::save( s );
  Q_INT8 b = m_bAllowAsDefault;
  Q_INT8 dst = (Q_INT8) m_DCOPServiceType;

  // Warning adding/removing fields here involves a binary incompatible change - update version 
  // number in ksycoca.h
  s << m_strType << m_strName << m_strExec << m_strIcon 
    << m_strTerminalOptions
    << m_strPath << m_strComment << m_lstServiceTypes << b << m_mapProps
    << m_strLibrary << m_libraryMajor << m_libraryMinor 
    << dst
    << m_strDesktopEntryPath << m_strDesktopEntryName;
}

bool KService::hasServiceType( const QString& _servicetype ) const
{
  if (!m_bValid) return false; // safety test

//  kDebugInfo( 7012, "Testing %s", m_strName.ascii());

//  QStringList::ConstIterator it = m_lstServiceTypes.begin();
//  for( ; it != m_lstServiceTypes.end(); ++it )
//    kDebugInfo( 7012, "    has %s", (*it).ascii() );

  // TODO : what about "all", "allfiles" and "alldirs" ?
  return ( m_lstServiceTypes.find( _servicetype ) != m_lstServiceTypes.end() );
}

QVariant KService::property( const QString& _name ) const
{
  if ( _name == "Type" )
    return QVariant( m_strType );
  else if ( _name == "Name" )
    return QVariant( m_strName );
  else if ( _name == "Exec" )
    return QVariant( m_strExec );
  else if ( _name == "Icon" )
    return QVariant( m_strIcon );
  else if ( _name == "TerminalOptions" )
    return QVariant( m_strTerminalOptions );
  else if ( _name == "Path" )
    return QVariant( m_strPath );
  else if ( _name == "Comment" )
    return QVariant( m_strComment );
  else if ( _name == "ServiceTypes" )
    return QVariant( m_lstServiceTypes );
  else if ( _name == "AllowAsDefault" )
    return QVariant( m_bAllowAsDefault );
  else if ( _name == "Library" )
    return QVariant( m_strLibrary );
  else if ( _name == "LibraryMajor" )
    return QVariant( m_libraryMajor );
  else if ( _name == "LibraryMinor" )
    return QVariant( m_libraryMinor );
  else if ( _name == "LibraryDependencies" )
    return QVariant( m_lstLibraryDeps );

  QMap<QString,QVariant>::ConstIterator it = m_mapProps.find( _name );
  if ( it == m_mapProps.end() )
    return QVariant(); // Invalid variant

  return it.data();
}

QStringList KService::propertyNames() const
{
  QStringList res;

  QMap<QString,QVariant>::ConstIterator it = m_mapProps.begin();
  for( ; it != m_mapProps.end(); ++it )
    res.append( it.key() );

  res.append( "Type" );
  res.append( "Name" );
  res.append( "Comment" );
  res.append( "Icon" );
  res.append( "Exec" );
  res.append( "TerminalOptions" );
  res.append( "Path" );
  res.append( "File" );
  res.append( "ServiceTypes" );
  res.append( "AllowAsDefault" );
  res.append( "Library" );
  res.append( "LibraryMajor" );
  res.append( "LibraryMinor" );
  res.append( "LibraryDependencies" );

  return res;
}

int
KService::startService( const QString &URL, QCString &dcopService, QString &error)
{
   return startServiceByDesktopPath( desktopEntryPath(), URL, dcopService, error);
}

KService::List KService::allServices()
{
  return KServiceFactory::self()->allServices();
}

KService::Ptr KService::service( const QString& _name )
{
  KService * s = KServiceFactory::self()->findServiceByName( _name );

  if (!s)
      s = KServiceFactory::self()->findServiceByDesktopName( _name );
  if (!s)
      s = KServiceFactory::self()->findServiceByDesktopPath( _name );
    
  return KService::Ptr( s );
}

KService::Ptr KService::serviceByName( const QString& _name )
{
  KService * s = KServiceFactory::self()->findServiceByName( _name );
  return KService::Ptr( s );
}

KService::Ptr KService::serviceByDesktopPath( const QString& _name )
{
  KService * s = KServiceFactory::self()->findServiceByDesktopPath( _name );
  return KService::Ptr( s );
}

KService::Ptr KService::serviceByDesktopName( const QString& _name )
{
  KService * s = KServiceFactory::self()->findServiceByDesktopName( _name );
  return KService::Ptr( s );
}

int
KService::startServiceByName( const QString& _name, const QString &URL,
                              QCString &dcopService, QString &error )
{
   return startServiceInternal( 
                      "start_service_by_name(QString,QString)", 
                      _name, URL, dcopService, error);
}

int
KService::startServiceByDesktopPath( const QString& _name, const QString &URL,
                              QCString &dcopService, QString &error )
{
   return startServiceInternal( 
                      "start_service_by_desktop_path(QString,QString)", 
                      _name, URL, dcopService, error);
}

int
KService::startServiceByDesktopName( const QString& _name, const QString &URL,
                              QCString &dcopService, QString &error )
{
   return startServiceInternal( 
                      "start_service_by_desktop_name(QString,QString)", 
                      _name, URL, dcopService, error);
}



int
KService::startServiceInternal( const QCString &function, 
                                const QString& _name, const QString &URL,
                                QCString &dcopService, QString &error )
{
   typedef struct serviceResult 
   {
      int result;
      QCString dcopName;
      QString error;
   };

   // Register app as able to send DCOP messages
   DCOPClient *dcopClient;
   if (kapp)
      dcopClient = kapp->dcopClient();
   else
      dcopClient = new DCOPClient;
   
   if (!dcopClient->isAttached())
   {
      if (!dcopClient->attach())
      {
         error = i18n("Could not register with DCOP.\n");
         return -1;
      }
   }
   QByteArray params;
   QDataStream stream(params, IO_WriteOnly);
   stream << _name << URL;
   QCString replyType;
   QByteArray replyData;
   if (!dcopClient->call("klauncher", "klauncher", 
	function, params, replyType, replyData))
   {
	error = i18n("KLauncher could not be reached via DCOP.\n");
        if (!kapp)
           delete dcopClient;
        return -1;
   }
   if (!kapp)
      delete dcopClient;

   QDataStream stream2(replyData, IO_ReadOnly);
   serviceResult result;
   stream2 >> result.result >> result.dcopName >> result.error;
   dcopService = result.dcopName;
   error = result.error;
   return result.result;
}

