/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include "kprotocolmanager.h"

#include <string.h>

#include <kstddirs.h>
#include <kglobal.h>
#include <kapp.h>
#include <kdebug.h>
#include <ksimpleconfig.h>
#include <qdir.h>
#include <qstrlist.h>
#include <kconfig.h>
#include <kstringhandler.h>

#undef Unsorted // X11 headers

KProtocolManager* KProtocolManager::s_pManager = 0L;

KProtocolManager::KProtocolManager()
{
  ASSERT( !s_pManager );

  s_pManager = this;

  QStringList list = KGlobal::dirs()->findDirs("config", "protocols");
  for (QStringList::ConstIterator it = list.begin(); it != list.end(); it++)
    scanConfig( *it );
}

void KProtocolManager::scanConfig( const QString& _dir )
{
  QDir dir( _dir, QString::null, QDir::Unsorted, QDir::Files | QDir::Readable );
  if (!dir.exists())
    return;
  QStringList p = dir.entryList();

  QStringList::Iterator e;
  for( e = p.begin(); e != p.end(); e++ )
  {
    if ( *e == "." || *e == ".." )
      continue;

    KSimpleConfig config( _dir + "/" + *e, true );
    config.setGroup( "Protocol" );

    Protocol p;
    p.exec = config.readEntry( "exec" );
    p.isSourceProtocol = config.readBoolEntry( "source", true );
    p.isHelperProtocol = config.readBoolEntry( "helper", false );
    p.supportsReading = config.readBoolEntry( "reading", false );
    p.supportsWriting = config.readBoolEntry( "writing", false );
    p.supportsMakeDir = config.readBoolEntry( "makedir", false );
    p.supportsDeleting = config.readBoolEntry( "deleting", false );
    p.supportsLinking = config.readBoolEntry( "linking", false );
    p.supportsMoving = config.readBoolEntry( "moving", false );
    p.listing = config.readListEntry( "listing" );
    p.supportsListing = ( p.listing.count() > 0 );
    p.defaultMimetype = config.readEntry( "defaultMimetype" );
    p.mimetypesExcludedFromFastMode = config.readListEntry( "mimetypesExcludedFromFastMode" );
    p.patternsExcludedFromFastMode = config.readListEntry( "patternsExcludedFromFastMode" );
    QString tmp = config.readEntry( "input" );
    if ( tmp == "filesystem" )
      p.inputType = T_FILESYSTEM;
    else if ( tmp == "stream" )
      p.inputType = T_STREAM;
    else
      p.inputType = T_NONE;
    tmp = config.readEntry( "output" );
    if ( tmp == "filesystem" )
      p.outputType = T_FILESYSTEM;
    else if ( tmp == "stream" )
      p.outputType = T_STREAM;
    else
      p.outputType = T_NONE;

    QString name = config.readEntry( "protocol" );

    m_protocols.insert( name, p );
  }
  // This is where we insert a small hack for FTP-Proxy.
  // If we want to use FTP-Proxy, then it's kio_http we want.
  if ( useProxy() && !ftpProxy().isEmpty() )
  {
      Iterator it = m_protocols.find( QString::fromLatin1("ftp") );
      if ( it != m_protocols.end() )
      {
        ConstIterator ithttp = m_protocols.find( QString::fromLatin1("http") );
        if ( ithttp != m_protocols.end() )
        {
          // Copy all the info about HTTP into the one about FTP
          it.data() = ithttp.data();
        }
      }
  }
}

bool KProtocolManager::isSourceProtocol( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return it.data().isSourceProtocol;
}

bool KProtocolManager::isFilterProtocol( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return !it.data().isSourceProtocol;
}

bool KProtocolManager::isHelperProtocol( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return it.data().isHelperProtocol;
}

bool KProtocolManager::isKnownProtocol( const QString& _protocol ) const
{
  return m_protocols.contains( _protocol );
}

bool KProtocolManager::supportsListing( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return it.data().supportsListing;
}

QStringList KProtocolManager::listing( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return QStringList();
  }

  return it.data().listing;
}

bool KProtocolManager::supportsReading( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return it.data().supportsReading;
}

bool KProtocolManager::supportsWriting( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return it.data().supportsWriting;
}

bool KProtocolManager::supportsMakeDir( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return it.data().supportsMakeDir;
}

bool KProtocolManager::supportsDeleting( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return it.data().supportsDeleting;
}

bool KProtocolManager::supportsLinking( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return it.data().supportsLinking;
}

bool KProtocolManager::supportsMoving( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  return it.data().supportsMoving;
}

QString KProtocolManager::defaultMimetype( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return QString::null;
  }

  return it.data().defaultMimetype;
}

bool KProtocolManager::mimetypeFastMode( const QString& _protocol, const QString & _mimetype ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  // return true if the exclude-list doesn't contain this mimetype
  return !(it.data().mimetypesExcludedFromFastMode.contains(_mimetype));
}

bool KProtocolManager::patternFastMode( const QString& _protocol, const QString & _filename ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return false;
  }

  // return true if the exclude-list doesn't contain this mimetype
  const QStringList & pat = it.data().patternsExcludedFromFastMode;
  for ( QStringList::ConstIterator pit = pat.begin(); pit != pat.end(); ++pit )
    if ( KStringHandler::matchFilename( _filename, *pit ) )
      return false; // in the list -> EXCLUDED

  return true; // not in the list -> ok
}

QStringList KProtocolManager::protocols() const
{
  QStringList res;

  ConstIterator it = m_protocols.begin();
  ConstIterator end = m_protocols.end();
  for (; it != end; ++it )
    res.append( it.key() );

  return res;
}

/*
QString KProtocolManager::library( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return QString::null;
  }

  return it.data().library;
}
*/

QString KProtocolManager::exec( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return QString::null;
  }

  return it.data().exec;
}

KProtocolManager::Type KProtocolManager::inputType( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return T_NONE;
  }

  return it.data().inputType;
}

KProtocolManager::Type KProtocolManager::outputType( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {
    kdError(127) << "Protocol " << _protocol << " not found" << endl;
    return T_NONE;
  }

  return it.data().outputType;
}

int KProtocolManager::readTimeout()
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( QString::null );
  return config.readNumEntry( "ReadTimeout", 15 ); // 15 seconds
}

bool KProtocolManager::markPartial()
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( QString::null );
  return config.readBoolEntry( "MarkPartial", true );
}

int KProtocolManager::minimumKeepSize()
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( QString::null );
  return config.readNumEntry( "MinimumKeepSize", 5000 ); // 5000 byte
}

bool KProtocolManager::autoResume()
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( QString::null );
  return config.readBoolEntry( "AutoResume", false );
}

bool KProtocolManager::persistentConnections()
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( QString::null );
  return config.readBoolEntry( "PersistentConnections", true );
}

bool KProtocolManager::useProxy()
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( "Proxy Settings" );
  return config.readBoolEntry( "UseProxy", false );
}

bool KProtocolManager::useCache()
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( "Cache Settings" );
  return config.readBoolEntry( "UseCache", true );
}

int KProtocolManager::maxCacheAge()
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( "Cache Settings" );
  return config.readNumEntry( "MaxCacheAge", 60*60*24*14 ); // 14 days
}

int KProtocolManager::maxCacheSize()
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( "Cache Settings" );
  return config.readNumEntry( "MaxCacheSize", 5000 );
}

QString KProtocolManager::ftpProxy()
{
    return proxyFor( "ftp" );
}

QString KProtocolManager::httpProxy()
{
    return proxyFor( "http" );
}

QString KProtocolManager::noProxyFor()
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( "Proxy Settings" );
  if( config.hasKey( "NoProxyFor" ) )
  {
    return config.readEntry( "NoProxyFor" );
  }
  config.setGroup( QString::null );
  return config.readEntry( "NoProxyFor" );
}

QString KProtocolManager::proxyFor( const QString& protocol )
{
  QString key = protocol.lower();
  KConfig config("kioslaverc", true, false);
  config.setGroup( "Proxy Settings" );

  // The following check is to ensure that we can read the old settings
  // incorrectly saved without a group and in the old format.  Once
  // setProxyFor(...) is invoked these settings would be correctly
  // saved under the appropriate group using the new format and the old
  // enteries if present will be deleted. This check will be unnecessary
  // after KDE 2.x is released and can be removed then.
  if( (key =="http" || key == "ftp") &&
     !config.hasKey( key + "Proxy" ) )
  {
    {
        if( key == "http" && config.hasKey( "HttpProxy" ) )
            return config.readEntry( "HttpProxy" );
        else if( key == "ftp" && config.hasKey( "HttpProxy" ) )
            return config.readEntry( "FtpProxy" );
    }
    if( key == "http" && config.hasKey( "HttpProxy" ) )
        return config.readEntry( "HttpProxy" );
    else if( key == "ftp" && config.hasKey( "HttpProxy" ) )
        return config.readEntry( "FtpProxy" );
  }

  return config.readEntry( key + "Proxy" );
}

QString KProtocolManager::remoteFileProtocol()
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( QString::null );
  return config.readEntry( "RemoteFileProtocol" );
}

void KProtocolManager::setReadTimeout( int _timeout )
{
  KConfig config("kioslaverc", false, false);
  config.setGroup( QString::null );
  config.writeEntry( "ReadTimeout", _timeout );
  config.sync();
}


void KProtocolManager::setMarkPartial( bool _mode )
{
  KConfig config("kioslaverc", false, false);
  config.setGroup( QString::null );
  config.writeEntry( "MarkPartial", _mode );
  config.sync();
}


void KProtocolManager::setMinimumKeepSize( int _size )
{
  KConfig config("kioslaverc", false, false);
  config.setGroup( QString::null );
  config.writeEntry( "MinimumKeepSize", _size );
  config.sync();
}


void KProtocolManager::setAutoResume( bool _mode )
{
  KConfig config("kioslaverc", false, false);
  config.setGroup( QString::null );
  config.writeEntry( "AutoResume", _mode );
  config.sync();
}


void KProtocolManager::setPersistentConnections( bool _mode )
{
  KConfig config("kioslaverc", false, false);
  config.setGroup( QString::null );
  config.writeEntry( "PersistentConnections", _mode );
  config.sync();
}

void KProtocolManager::setRemoteFileProtocol(const QString &remoteFileProtocol)
{
  KConfig config("kioslaverc", false, false);
  config.setGroup( QString::null );
  config.writeEntry( "RemoteFileProtocol", remoteFileProtocol );
  config.sync();
}

void KProtocolManager::setUseCache( bool _mode )
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( "Cache Settings" );
  config.writeEntry( "UseCache", _mode );
  config.sync();
}

void KProtocolManager::setMaxCacheSize( int cache_size )
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( "Cache Settings" );
  config.writeEntry( "MaxCacheSize", cache_size );
  config.sync();
}

void KProtocolManager::setMaxCacheAge( int cache_age )
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( "Cache Settings" );
  config.writeEntry( "MaxCacheAge", cache_age );
  config.sync();
}

void KProtocolManager::setUseProxy( bool _mode )
{
  KSimpleConfig config("kioslaverc", false );
  config.setGroup( QString::null );
  if( config.hasKey( "UseProxy" ) )
    config.deleteEntry( "UseProxy", true );
  config.setGroup( "Proxy Settings" );
  config.writeEntry( "UseProxy", _mode );
  config.sync();
}

void KProtocolManager::setFtpProxy( const QString& _proxy )
{
    setProxyFor( "ftp", _proxy );
}

void KProtocolManager::setHttpProxy( const QString& _proxy )
{
    setProxyFor( "http", _proxy );
}

void KProtocolManager::setNoProxyFor( const QString& _noproxy )
{
  KSimpleConfig config( "kioslaverc", false );
  config.setGroup( QString::null );
  if( config.hasKey( "NoProxyFor" ) )
    config.deleteEntry( "NoProxyFor", true );
  config.setGroup( "Proxy Settings" );
  config.writeEntry( "NoProxyFor", _noproxy );
  config.sync();
}

void KProtocolManager::setProxyFor( const QString& protocol, const QString& _proxy )
{
  QString key = protocol.lower();
  KSimpleConfig config( "kioslaverc", false );
  config.setGroup( "Proxy Settings" );
  // The following check is to ensure that we properly remove the
  // the settings that were saved in an incorrect format and group.
  // This check will be unnecessary after KDE 2.x is released and
  // can be safely removed then.
  if( key == "http" || key == "ftp" )
  {
    {
        // deletes old enteries saved without group
        KConfigGroupSaver saver( &config, QString::null );
        if( config.hasKey( "HttpProxy" ) )
          config.deleteEntry( "HttpProxy", true );
        else if( config.hasKey( "FtpProxy" ) )
          config.deleteEntry( "FtpProxy", true );
    }
    // deletes old enteries saved under "Proxy Settings"
    // but used the old format "HttpProxy" instead of the
    // new one "httpProxy".
    if( config.hasKey( "HttpProxy" ) )
        config.deleteEntry( "HttpProxy", true );
    else if( config.hasKey( "FtpProxy" ) )
        config.deleteEntry( "FtpProxy", true );
  }
  config.writeEntry( key + "Proxy", _proxy );
  config.sync();
}
