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

#include "kservice.h"
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
#include "kservicetype.h"
#include "kuserprofile.h"
#include "ksycoca.h"
#include <kdesktopfile.h>

KService::KService( const QString & _fullpath )
 : KSycocaEntry( _fullpath)
{
  KDesktopFile config( _fullpath );

  init(&config);
}

KService::KService( KDesktopFile *config )
 : KSycocaEntry( config->filename())
{
  init(config);
}

void
KService::init( KDesktopFile *config )
{
  m_bValid = true;

  bool absPath = (entryPath()[0] == '/');

  config->setDesktopGroup();
  config->setDollarExpansion( true ); // mainly for Exec and Path
  m_strType = config->readEntry( "Type" );
  if ( m_strType.isEmpty() )
  {
    /*kdWarning(7012) << "The desktop entry file " << entryPath()
                    << " has no Type=... entry."
                    << " It should be \"Application\" or \"Service\"" << endl;
    m_bValid = false;
    return;*/
    m_strType = "Application";
  } else if ( m_strType != "Application" && m_strType != "Service" )
  {
    kdWarning(7012) << "The desktop entry file " << entryPath()
                    << " has Type=" << m_strType
                    << " instead of \"Application\" or \"Service\"" << endl;
    m_bValid = false;
    return;
  }

  // check if TryExec is valid
  if (!config->tryExec()) {
      m_bValid = false;
      return;
  }

  QString resource = config->resource();

  if ( (m_strType == "Application") &&
       (!resource.isEmpty()) &&
       (resource != "apps") &&
       !absPath)
  {
    kdWarning(7012) << "The desktop entry file " << entryPath()
           << " has Type=" << m_strType << "but is located under \"" << resource
           << "\" instead of \"apps\"" << endl;
    m_bValid = false;
    return;
  }

  if ( (m_strType == "Service") &&
       (!resource.isEmpty()) &&
       (resource != "services") &&
       !absPath)
  {
    kdWarning(7012) << "The desktop entry file " << entryPath()
           << " has Type=" << m_strType << "but is located under \"" << resource
           << "\" instead of \"services\"" << endl;
    m_bValid = false;
    return;
  }

  QString name = entryPath();
  int pos = name.findRev('/');
  if (pos != -1)
     name = name.mid(pos+1);
  pos = name.find('.');
  if (pos != -1)
     name = name.left(pos);

  m_strExec = config->readEntry( "Exec" );
  m_strName = config->readEntry( "Name" );
  if ( m_strName.isEmpty() )
  {
    kdWarning(7012) << "The desktop entry file " << entryPath()
                    <<  " has no Name" << endl;
    m_strName = name;
  }

  m_strIcon = config->readEntry( "Icon", "unknown" );
  m_bTerminal = (config->readBoolEntry( "Terminal" )); // should be a property IMHO
  m_strTerminalOptions = config->readEntry( "TerminalOptions" ); // should be a property IMHO
  m_strPath = config->readEntry( "Path" );
  m_strComment = config->readEntry( "Comment" );
  m_bDeleted = config->readBoolEntry( "Hidden", false );
  m_lstKeywords = config->readListEntry("Keywords");
  m_strLibrary = config->readEntry( "X-KDE-Library" );
  m_strInit = config->readEntry("X-KDE-Init" );
  m_libraryMajor = config->readNumEntry( "X-KDE-LibraryMajor", 0 );
  m_libraryMinor = config->readNumEntry( "X-KDE-LibraryMinor", 0 );
  m_lstLibraryDeps = config->readListEntry( "X-KDE-LibraryDependencies" );
  m_lstServiceTypes = config->readListEntry( "ServiceTypes" );
  // For compatibility with KDE 1.x
  m_lstServiceTypes += config->readListEntry( "MimeType", ';' );
  // Now in application.desktop, i.e. read as properties
  //m_docPath = config->readEntry("DocPath");
  //m_bHideFromPanel = config->readBoolEntry("X-KDE-HideFromPanel");
  //m_bSuid = (config->readEntry( "X-KDE-SubstituteUID" ) == "1");
  //m_strUsername = config->readEntry( "X-KDE-Username" );

  if ( m_strType == "Application" && !m_lstServiceTypes.contains("Application") )
    // Applications implement the service type "Application" ;-)
    m_lstServiceTypes += "Application";

  QString dcopServiceType = config->readEntry("X-DCOP-ServiceType").lower();
  if (dcopServiceType == "unique")
     m_DCOPServiceType = DCOP_Unique;
  else if (dcopServiceType == "multi")
     m_DCOPServiceType = DCOP_Multi;
  else
     m_DCOPServiceType = DCOP_None;

  m_strDesktopEntryName = name.lower();

  m_bAllowAsDefault = config->readBoolEntry( "AllowDefault", true );

  m_initialPreference = config->readNumEntry( "InitialPreference", 1 );

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
        QVariant v = config->readPropertyEntry( pit.key(), pit.data() );
        if ( v.isValid() )
          m_mapProps.insert( pit.key(), v );
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

QPixmap KService::pixmap( int _group, int _force_size, int _state, QString * _path ) const
{
  return KGlobal::iconLoader()->loadIcon( m_strIcon, _group, _force_size, _state, _path );
}

void KService::load( QDataStream& s )
{
  // dummies are here because of fields that were removed, to keep bin compat.
  // Feel free to re-use, but fields for Applications only (not generic services)
  // should rather be added to application.desktop
  Q_INT8 def, term, dummy1, dummy2;
  Q_INT8 dst, initpref;
  QString dummyStr1, dummyStr2;

  s >> m_strType >> m_strName >> m_strExec >> m_strIcon
    >> term >> m_strTerminalOptions
    >> m_strPath >> m_strComment >> m_lstServiceTypes >> def >> m_mapProps
    >> m_strLibrary >> m_libraryMajor >> m_libraryMinor
    >> dst
    >> m_strDesktopEntryName
    >> dummy1 >> dummyStr1 >> initpref >> dummyStr2 >> dummy2 
    >> m_lstKeywords >> m_strInit;

  m_bAllowAsDefault = def;
  m_bTerminal = term;
  m_DCOPServiceType = (DCOPServiceType_t) dst;
  m_initialPreference = initpref;

  m_bValid = true;
}

void KService::save( QDataStream& s )
{
  KSycocaEntry::save( s );
  Q_INT8 def = m_bAllowAsDefault, initpref = m_initialPreference;
  Q_INT8 term = m_bTerminal;
  Q_INT8 dst = (Q_INT8) m_DCOPServiceType;
  Q_INT8 dummy1 = 0, dummy2 = 0; // see ::load
  QString dummyStr1, dummyStr2;

  // !! This data structure should remain binary compatible at all times !!
  // You may add new fields at the end. Make sure to update the version
  // number in ksycoca.h
  s << m_strType << m_strName << m_strExec << m_strIcon
    << term << m_strTerminalOptions
    << m_strPath << m_strComment << m_lstServiceTypes << def << m_mapProps
    << m_strLibrary << m_libraryMajor << m_libraryMinor
    << dst
    << m_strDesktopEntryName
    << dummy1 << dummyStr1 << initpref << dummyStr2 << dummy2
    << m_lstKeywords << m_strInit;
}

bool KService::hasServiceType( const QString& _servicetype ) const
{
  if (!m_bValid) return false; // safety test

//  kdDebug(7012) << "Testing " << m_strName << endl;

//  QStringList::ConstIterator it = m_lstServiceTypes.begin();
//  for( ; it != m_lstServiceTypes.end(); ++it )
//    kdDebug(7012) << "    has " << (*it) << endl;

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
  else if ( _name == "Terminal" )
    return QVariant( static_cast<int>(m_bTerminal) );
  else if ( _name == "TerminalOptions" )
    return QVariant( m_strTerminalOptions );
  else if ( _name == "Path" )
    return QVariant( m_strPath );
  else if ( _name == "Comment" )
    return QVariant( m_strComment );
  else if ( _name == "ServiceTypes" )
    return QVariant( m_lstServiceTypes );
  else if ( _name == "AllowAsDefault" )
    return QVariant( static_cast<int>(m_bAllowAsDefault) );
  else if ( _name == "InitialPreference" )
    return QVariant( m_initialPreference );
  else if ( _name == "Library" )
    return QVariant( m_strLibrary );
  else if ( _name == "LibraryMajor" )
    return QVariant( m_libraryMajor );
  else if ( _name == "LibraryMinor" )
    return QVariant( m_libraryMinor );
  else if ( _name == "LibraryDependencies" )
    return QVariant( m_lstLibraryDeps );
  else if ( _name == "DesktopEntryPath" )
    return QVariant( entryPath() );
  else if ( _name == "DesktopEntryName")
    return QVariant( m_strDesktopEntryName );

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
  res.append( "Terminal" );
  res.append( "TerminalOptions" );
  res.append( "Path" );
  res.append( "File" );
  res.append( "ServiceTypes" );
  res.append( "AllowAsDefault" );
  res.append( "InitialPreference" );
  res.append( "Library" );
  res.append( "LibraryMajor" );
  res.append( "LibraryMinor" );
  res.append( "LibraryDependencies" );
  res.append( "DesktopEntryPath" );
  res.append( "DesktopEntryName" );

  return res;
}

KService::List KService::allServices()
{
  return KServiceFactory::self()->allServices();
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

KService::List KService::allInitServices()
{
  return KServiceFactory::self()->allInitServices();
}

bool KService::substituteUid() const {
  QVariant v = property("X-KDE-SubstituteUID");
  return v.isValid() && v.toBool();
}
QString KService::username() const {
  QVariant v = property("X-KDE-Username");
  return v.isValid() ? v.toString() : QString::null;
}

