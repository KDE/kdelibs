#include "kprotocolmanager.h"

#include <string.h>

#include <kapp.h>
#include <kdebug.h>
#include <ksimpleconfig.h>
#include <qdir.h>
#include <qstrlist.h>

KProtocolManager* KProtocolManager::s_pManager = 0L;

KProtocolManager::KProtocolManager()
{
  ASSERT( !s_pManager );
  
  s_pManager = this;

  scanConfig( KApplication::kde_configdir() + "/protocols", FALSE );

  scanConfig( KApplication::localconfigdir() + "/protocols", TRUE );
}

void KProtocolManager::scanConfig( const QString& _dir, bool _islocal )
{
  QDir dir( _dir );
  const QStrList* p = dir.entryList();
  if ( !p )
    return;
    
  QStrList lst( *p );
  
  const char *e;
  for( e = lst.first(); e; e = lst.next() )
  {
    if ( strcmp( e, "." ) == 0 || strcmp( e, ".." ) == 0 )
      continue;

    KSimpleConfig config( _dir + "/" + e, TRUE );
    config.setGroup( "Protocol" );
  
    Protocol p;
    QString exec = config.readEntry( "exec" );
    if ( _islocal )
      p.executable = KApplication::localkdedir() + "/" + exec;
    else
      p.executable = KApplication::kde_bindir() + "/" + exec;
    p.isSourceProtocol = config.readBoolEntry( "source", TRUE );
    p.supportsReading = config.readBoolEntry( "reading", FALSE );
    p.supportsWriting = config.readBoolEntry( "writing", FALSE );
    p.supportsMakeDir = config.readBoolEntry( "makedir", FALSE );
    p.supportsDeleting = config.readBoolEntry( "deleting", FALSE );
    p.supportsLinking = config.readBoolEntry( "linking", FALSE );
    p.supportsMoving = config.readBoolEntry( "moving", FALSE );
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
    return FALSE;
  }
  
  return it.data().isSourceProtocol;
}

bool KProtocolManager::isFilterProtocol( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {    
    kdebug( KDEBUG_ERROR, 7008, "Protocol %s not found", _protocol.ascii() );
    return FALSE;
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
    return FALSE;
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
    return FALSE;
  }
  
  return it.data().supportsReading;
}

bool KProtocolManager::supportsWriting( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {    
    kdebug( KDEBUG_ERROR, 7008, "Protocol %s not found", _protocol.ascii() );
    return FALSE;
  }
  
  return it.data().supportsWriting;
}

bool KProtocolManager::supportsMakeDir( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {    
    kdebug( KDEBUG_ERROR, 7008, "Protocol %s not found", _protocol.ascii() );
    return FALSE;
  }
  
  return it.data().supportsMakeDir;
}

bool KProtocolManager::supportsDeleting( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {    
    kdebug( KDEBUG_ERROR, 7008, "Protocol %s not found", _protocol.ascii() );
    return FALSE;
  }
  
  return it.data().supportsDeleting;
}

bool KProtocolManager::supportsLinking( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {    
    kdebug( KDEBUG_ERROR, 7008, "Protocol %s not found", _protocol.ascii() );
    return FALSE;
  }
  
  return it.data().supportsLinking;
}

bool KProtocolManager::supportsMoving( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {    
    kdebug( KDEBUG_ERROR, 7008, "Protocol %s not found", _protocol.ascii() );
    return FALSE;
  }
  
  return it.data().supportsMoving;
}

QString KProtocolManager::executable( const QString& _protocol ) const
{
  ConstIterator it = m_protocols.find( _protocol );
  if ( it == m_protocols.end() )
  {    
    kdebug( KDEBUG_ERROR, 7008, "Protocol %s not found", _protocol.ascii() );
    return QString();
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

int KProtocolManager::getReadTimeout() const
{
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		 KApplication::localconfigdir() + "/kioslaverc" );

  return config.readNumEntry( "ReadTimeout", 15 ); // 15 seconds
}

bool KProtocolManager::getMarkPartial() const
{
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		 KApplication::localconfigdir() + "/kioslaverc" );
  
  return config.readBoolEntry( "MarkPartial", TRUE );
}

int KProtocolManager::getMinimumKeepSize() const
{
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		 KApplication::localconfigdir() + "/kioslaverc" );

  return config.readNumEntry( "MinimumKeepSize", 5000 ); // 5000 bytes
}

bool KProtocolManager::getAutoResume() const
{
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		 KApplication::localconfigdir() + "/kioslaverc" );

  return config.readBoolEntry( "AutoResume", FALSE );
}

bool KProtocolManager::getPersistent() const
{
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		 KApplication::localconfigdir() + "/kioslaverc" );

  return config.readBoolEntry( "PersistentConnections", TRUE );
}

bool KProtocolManager::getUseProxy() const
{
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		 KApplication::localconfigdir() + "/kioslaverc" );

  return config.readBoolEntry( "UseProxy", false );
}

QString KProtocolManager::getFtpProxy() const
{
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		 KApplication::localconfigdir() + "/kioslaverc" );

  return config.readEntry( "FtpProxy" );
}

QString KProtocolManager::getHttpProxy() const
{
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		  KApplication::localconfigdir() + "/kioslaverc" );

  return config.readEntry( "HttpProxy" );
}

QString KProtocolManager::getNoProxyFor() const
{
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		  KApplication::localconfigdir() + "/kioslaverc" );

  return config.readEntry( "NoProxyFor" );
}

void KProtocolManager::setReadTimeout( int _timeout )
{
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		 KApplication::localconfigdir() + "/kioslaverc" );

  config.writeEntry( "ReadTimeout", _timeout );
  
  config.sync();
}


void KProtocolManager::setMarkPartial( bool _mode )
{
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		  KApplication::localconfigdir() + "/kioslaverc" );
  
  config.writeEntry( "MarkPartial", _mode );
  
  config.sync();
}


void KProtocolManager::setMinimumKeepSize( int _size )
{
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		 KApplication::localconfigdir() + "/kioslaverc" );
  
  config.writeEntry( "MinimumKeepSize", _size );
  
  config.sync();
}


void KProtocolManager::setAutoResume( bool _mode )
{
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		 KApplication::localconfigdir() + "/kioslaverc" );
  
  config.writeEntry( "AutoResume", _mode );
  
  config.sync();
}


void KProtocolManager::setPersistent( bool _mode )
{
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		 KApplication::localconfigdir() + "/kioslaverc" );
  
  config.writeEntry( "PersistentConnections", _mode );
  
  config.sync();
}


void KProtocolManager::setUseProxy( bool _mode )
{
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		 KApplication::localconfigdir() + "/kioslaverc" );
  
  config.writeEntry( "UseProxy", _mode );

  config.sync();
}


void KProtocolManager::setFtpProxy( const QString& _proxy )
{
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		 KApplication::localconfigdir() + "/kioslaverc" );

  config.writeEntry( "FtpProxy", _proxy );

  config.sync();
}


void KProtocolManager::setHttpProxy( const QString& _proxy )
{
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		  KApplication::localconfigdir() + "/kioslaverc" );
  
  config.writeEntry( "HttpProxy", _proxy );
  
  config.sync();
}


void KProtocolManager::setNoProxyFor( const QString& _noproxy )
{
  KConfig config(KApplication::kde_configdir() + "/kioslaverc",
		 KApplication::localconfigdir() + "/kioslaverc" );
  
  config.writeEntry( "NoProxyFor", _noproxy );

  config.sync();
}
