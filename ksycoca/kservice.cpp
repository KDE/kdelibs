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

#include <kservicefactory.h>
#include "kservice.h"
#include "kservicetype.h"
#include "kuserprofile.h"

#include <unistd.h>
#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <assert.h>

#include <qstring.h>
#include <qmsgbox.h>
#include <qtl.h>

#include <ksimpleconfig.h>
#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>

KService::KService( const QString& _name, const QString& _exec, const QString &_corbaexec,
                    const QString& _icon, const QStringList& _lstServiceTypes,
		    const QString& _comment, bool _allow_as_default,
		    const QString& _path, const QString& _terminal,
		    const QString&, const QString& _act_mode,
		    const QStringList& _repo_ids,
		    const QString& _lib, int _minor, int _major, const QStringList& deps )
{
  m_bValid = true;

  m_strName = _name;
  m_strExec = _exec;
  m_strCORBAExec = _corbaexec;
  m_strIcon = _icon;
  m_strComment = _comment;
  m_lstServiceTypes = _lstServiceTypes;
  m_strPath = _path;
  m_strTerminalOptions = _terminal;
  m_bAllowAsDefault = _allow_as_default;
  m_strActivationMode = _act_mode;
  m_lstRepoIds = _repo_ids;
  m_strLibrary = _lib;
  m_libraryMajor = _major;
  m_libraryMinor = _minor;
  m_lstLibraryDeps = deps;
}

/*
KService::KService()
{
  m_bValid = false;
}
*/

KService::KService( KSimpleConfig& config )
{
  m_bValid = true;

  config.setDesktopGroup();
  m_strExec = config.readEntry( "Exec" );
  m_strName = config.readEntry( "Name" );
  if ( m_strName.isEmpty() )
  {
    m_bValid = false;
    return;
  }

  m_strCORBAExec = config.readEntry( "CORBAExec" ); //should we use X-KDE-CORBAExec instead?? (Simon)
  m_strIcon = config.readEntry( "Icon", "unknown.xpm" );
  m_strTerminalOptions = config.readEntry( "TerminalOptions" );
  m_strPath = config.readEntry( "Path" );
  m_strComment = config.readEntry( "Comment" );
  m_strActivationMode = config.readEntry( "X-KDE-ActivationMode", "UNIX" );
  m_lstRepoIds = config.readListEntry( "X-KDE-RepoIds" );
  m_strLibrary = config.readEntry( "X-KDE-Library" );
  m_libraryMajor = config.readNumEntry( "X-KDE-LibraryMajor", 0 );
  m_libraryMinor = config.readNumEntry( "X-KDE-LibraryMinor", 0 );
  m_lstLibraryDeps = config.readListEntry( "X-KDE-LibraryDependencies" );
  m_lstServiceTypes = config.readListEntry( "ServiceTypes" );
  // For compatibility with KDE 1.x
  m_lstServiceTypes += config.readListEntry( "MimeType", ';' );

  m_bAllowAsDefault = config.readBoolEntry( "AllowDefault" );

  // Load all additional properties
  QStringList::Iterator it = m_lstServiceTypes.begin();
  for( ; it != m_lstServiceTypes.end(); ++it )
  {
    KServiceType* s = KServiceType::serviceType( *it );
    if ( s )
    {
      const QMap<QString,QVariant::Type>& pd = s->propertyDefs();
      QMap<QString,QVariant::Type>::ConstIterator pit = pd.begin();
      for( ; pit != pd.end(); ++pit )
      {
	m_mapProps.insert( pit.key(), config.readPropertyEntry( pit.key(), pit.data() ) );
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
}

void KService::load( QDataStream& s )
{
  Q_INT8 b;

  s >> m_strName >> m_strExec >> m_strCORBAExec >> m_strIcon >> m_strTerminalOptions
    >> m_strPath >> m_strComment >> m_lstServiceTypes >> b >> m_mapProps
    >> m_strActivationMode >> m_strLibrary >> m_libraryMajor >> m_libraryMinor >> m_lstRepoIds;
  m_bAllowAsDefault = b;

  m_bValid = true;
}

void KService::save( QDataStream& s )
{
  KSycocaEntry::save( s );
  Q_INT8 b = m_bAllowAsDefault;

  s << m_strName << m_strExec << m_strCORBAExec << m_strIcon << m_strTerminalOptions
    << m_strPath << m_strComment << m_lstServiceTypes << b << m_mapProps
    << m_strActivationMode << m_strLibrary << m_libraryMajor << m_libraryMinor << m_lstRepoIds;
}

bool KService::hasServiceType( const QString& _servicetype ) const
{
  if (!m_bValid) return false; // safety test

//  kdebug(KDEBUG_INFO, 7012, "Testing %s", m_strName.ascii());

//  QStringList::ConstIterator it = m_lstServiceTypes.begin();
//  for( ; it != m_lstServiceTypes.end(); ++it )
//    kdebug(KDEBUG_INFO, 7012, "    has %s", (*it).ascii() );

  // TODO : what about "all", "allfiles" and "alldirs" ?
  return ( m_lstServiceTypes.find( _servicetype ) != m_lstServiceTypes.end() );
}

KService::PropertyPtr KService::property( const QString& _name ) const
{
  QVariant* p = 0;

  if ( _name == "Name" )
    p = new QVariant( m_strName );
  else if ( _name == "Exec" )
    p = new QVariant( m_strExec );
  else if ( _name == "CORBAExec" )
    p = new QVariant( m_strCORBAExec );
  else if ( _name == "Icon" )
    p = new QVariant( m_strIcon );
  else if ( _name == "TerminalOptions" )
    p = new QVariant( m_strTerminalOptions );
  else if ( _name == "Path" )
    p = new QVariant( m_strPath );
  else if ( _name == "Comment" )
    p = new QVariant( m_strComment );
  else if ( _name == "ActivationMode" )
    p = new QVariant( m_strActivationMode );
  else if ( _name == "RepoIds" )
    p = new QVariant( m_lstRepoIds );
  else if ( _name == "ServiceTypes" )
    p = new QVariant( m_lstServiceTypes );
  else if ( _name == "AllowAsDefault" )
    p = new QVariant( m_bAllowAsDefault );
  else if ( _name == "Library" )
    p = new QVariant( m_strLibrary );
  else if ( _name == "LibraryMajor" )
    p = new QVariant( m_libraryMajor );
  else if ( _name == "LibraryMinor" )
    p = new QVariant( m_libraryMinor );
  else if ( _name == "LibraryDependencies" )
    p = new QVariant( m_lstLibraryDeps );
		
  if ( p )
    return KService::PropertyPtr( p );

  QMap<QString,QVariant>::ConstIterator it = m_mapProps.find( _name );
  if ( it == m_mapProps.end() )
    return (QVariant*)0;

  p = (QVariant*)(&(it.data()));

  p->ref();
  return KService::PropertyPtr( p );
}

QStringList KService::propertyNames() const
{
  QStringList res;

  QMap<QString,QVariant>::ConstIterator it = m_mapProps.begin();
  for( ; it != m_mapProps.end(); ++it )
    res.append( it.key() );

  res.append( "Name" );
  res.append( "Comment" );
  res.append( "Icon" );
  res.append( "Exec" );
  res.append( "CORBAExec" );
  res.append( "TerminalOptions" );
  res.append( "Path" );
  res.append( "File" );
  res.append( "ServiceTypes" );
  res.append( "AllowAsDefault" );
  res.append( "RepoIds" );
  res.append( "ActivationMode" );
  res.append( "Library" );
  res.append( "LibraryMajor" );
  res.append( "LibraryMinor" );
  res.append( "LibraryDependencies" );

  return res;
}

/*
QDataStream& operator>>( QDataStream& _str, KService& s )
{
  s.load( _str );
  return _str;
}

QDataStream& operator<<( QDataStream& _str, KService& s )
{
  s.save( _str );
  return _str;
}
*/

KServiceList* KService::allServices()
{
  return KServiceFactory::allServices();
}

KService* KService::service( const QString& _name )
{
  return KServiceFactory::findServiceByName( _name );
}

