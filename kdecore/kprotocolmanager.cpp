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
  QDir dir( _dir );
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
    QString exec = config.readEntry( "exec" );
    if (!exec.isEmpty())
	p.executable = locate("exe", exec);
    else 
	p.executable = "";
    p.isSourceProtocol = config.readBoolEntry( "source", true );
    p.supportsReading = config.readBoolEntry( "reading", false );
    p.supportsWriting = config.readBoolEntry( "writing", false );
    p.supportsMakeDir = config.readBoolEntry( "makedir", false );
    p.supportsDeleting = config.readBoolEntry( "deleting", false );
    p.supportsLinking = config.readBoolEntry( "linking", false );
    p.supportsMoving = config.readBoolEntry( "moving", false );
    QStrList lst;
    config.readListEntry( "listing", lst );
    p.supportsListing = ( lst.count() > 0 );
    const char *l;
    for( l = lst.first(); l; l = lst.next() )
      p.listing.append( l );
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
}

bool KProtocolManager::isSourceProtocol( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {    
    kdebug( KDEBUG_ERROR, 7008, "Protocol %s not found", _protocol.ascii() );
    return false;
  }
  
  return it.data().isSourceProtocol;
}

bool KProtocolManager::isFilterProtocol( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {    
    kdebug( KDEBUG_ERROR, 7008, "Protocol %s not found", _protocol.ascii() );
    return false;
  }
  
  return !it.data().isSourceProtocol;
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
    kdebug( KDEBUG_ERROR, 7008, "Protocol %s not found", _protocol.ascii() );
    return false;
  }
  
  return it.data().supportsListing;
}

QStringList KProtocolManager::listing( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {    
    kdebug( KDEBUG_ERROR, 7008, "Protocol %s not found", _protocol.ascii() );
    return QStringList();
  }
  
  return it.data().listing;
}

bool KProtocolManager::supportsReading( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {    
    kdebug( KDEBUG_ERROR, 7008, "Protocol %s not found", _protocol.ascii() );
    return false;
  }
  
  return it.data().supportsReading;
}

bool KProtocolManager::supportsWriting( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {    
    kdebug( KDEBUG_ERROR, 7008, "Protocol %s not found", _protocol.ascii() );
    return false;
  }
  
  return it.data().supportsWriting;
}

bool KProtocolManager::supportsMakeDir( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {    
    kdebug( KDEBUG_ERROR, 7008, "Protocol %s not found", _protocol.ascii() );
    return false;
  }
  
  return it.data().supportsMakeDir;
}

bool KProtocolManager::supportsDeleting( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {    
    kdebug( KDEBUG_ERROR, 7008, "Protocol %s not found", _protocol.ascii() );
    return false;
  }
  
  return it.data().supportsDeleting;
}

bool KProtocolManager::supportsLinking( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {    
    kdebug( KDEBUG_ERROR, 7008, "Protocol %s not found", _protocol.ascii() );
    return false;
  }
  
  return it.data().supportsLinking;
}

bool KProtocolManager::supportsMoving( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {    
    kdebug( KDEBUG_ERROR, 7008, "Protocol %s not found", _protocol.ascii() );
    return false;
  }
  
  return it.data().supportsMoving;
}

QString KProtocolManager::executable( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {    
    kdebug( KDEBUG_ERROR, 7008, "Protocol %s not found", _protocol.ascii() );
    return QString::null;
  }
  
  return it.data().executable;
}

KProtocolManager::Type KProtocolManager::inputType( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {    
    kdebug( KDEBUG_ERROR, 7008, "Protocol %s not found", _protocol.ascii() );
    return T_NONE;
  }
  
  return it.data().inputType;
}

KProtocolManager::Type KProtocolManager::outputType( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {    
    kdebug( KDEBUG_ERROR, 7008, "Protocol %s not found", _protocol.ascii() );
    return T_NONE;
  }
  
  return it.data().outputType;
}

int KProtocolManager::readTimeout() const
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( QString::null );
  return config.readNumEntry( "ReadTimeout", 15 ); // 15 seconds
}

bool KProtocolManager::markPartial() const
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( QString::null );
  return config.readBoolEntry( "MarkPartial", true );
}

int KProtocolManager::minimumKeepSize() const
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( QString::null );
  return config.readNumEntry( "MinimumKeepSize", 5000 ); // 5000 bytey
}

bool KProtocolManager::autoResume() const
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( QString::null );
  return config.readBoolEntry( "AutoResume", false );
}

bool KProtocolManager::persistentConnections() const
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( QString::null );
  return config.readBoolEntry( "PersistentConnections", true );
}

bool KProtocolManager::useProxy() const
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( "Proxy Settings" );
  return config.readBoolEntry( "UseProxy", false );
}

QString KProtocolManager::ftpProxy() const
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( "Proxy Settings" );
  return config.readEntry( "FtpProxy" );
}

QString KProtocolManager::httpProxy() const
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( "Proxy Settings" );
  return config.readEntry( "HttpProxy" );
}

QString KProtocolManager::noProxyFor() const
{
  KConfig config("kioslaverc", true, false);
  config.setGroup( "Proxy Settings" );
  return config.readEntry( "NoProxyFor" );
}

QString KProtocolManager::remoteFileProtocol() const
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


void KProtocolManager::setUseProxy( bool _mode )
{
  KConfig config("kioslaverc", false, false);
  config.setGroup( QString::null );
  config.writeEntry( "UseProxy", _mode );
  config.sync();
}


void KProtocolManager::setFtpProxy( const QString& _proxy )
{
  KConfig config("kioslaverc", false, false);
  config.setGroup( QString::null );
  config.writeEntry( "FtpProxy", _proxy );
  config.sync();
}


void KProtocolManager::setHttpProxy( const QString& _proxy )
{
  KConfig config("kioslaverc", false, false);
  config.setGroup( QString::null );
  config.writeEntry( "HttpProxy", _proxy );
  config.sync();
}


void KProtocolManager::setNoProxyFor( const QString& _noproxy )
{
  KConfig config("kioslaverc", false, false);
  config.setGroup( QString::null );
  config.writeEntry( "NoProxyFor", _noproxy );
  config.sync();
}

void KProtocolManager::setRemoteFileProtocol(const QString &remoteFileProtocol)
{
  KConfig config("kioslaverc", false, false);
  config.setGroup( QString::null );
  config.writeEntry( "RemoteFileProtocol", remoteFileProtocol );
  config.sync();
}
